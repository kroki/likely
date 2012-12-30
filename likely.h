/*
  Copyright (C) 2009, 2012 Tomash Brechko.  All rights reserved.

  This header is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This header is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this header.  If not, see <http://www.gnu.org/licenses/>.


  DESCRIPTION:

  Define likely() and unlikely() macros to guide branch prediction.
  Implementation requires GNU tool-chain (GCC, GNU ld).

  Optional -D KROKI_LIKELY_REPORT[=RATIO] during compilation will
  enable prediction analysis.  RATIO should be a float between 0.0 and
  1.0, default is 1.0.  Report on likely()/unlikely() with prediction
  rate less than or equal to RATIO will be printed to stderr on
  program exit.  Prediction accounting is not thread-safe: when
  several threads try to increment a counter simultaneously some
  increments may be lost (however this should be a rare event and
  should not affect overall statistics).

  Defining KROKI_LIKELY_NOPOLLUTE will result in omitting
  likely()/unlikely() alias definition (functionality will still be
  available as kroki_likely()/kroki_unlikely()).

  Inspired by Ulrich Drepper's "What Every Programmer Should Know About Memory"
  (http://www.akkadia.org/drepper/cpumemory.pdf) annex A.2.
*/

#ifndef KROKI_LIKELY_NOPOLLUTE

#define likely(expr)  kroki_likely(expr)
#define unlikely(expr)  kroki_unlikely(expr)

#endif  /* ! KROKI_LIKELY_NOPOLLUTE */


#ifndef KROKI_LIKELY_H
#define KROKI_LIKELY_H 1


#ifndef KROKI_LIKELY_REPORT

#define kroki_likely(expr)  __builtin_expect(!!(expr), 1)
#define kroki_unlikely(expr)  __builtin_expect(!!(expr), 0)

#else  /* KROKI_LIKELY_REPORT */

#include <stdio.h>


struct _kroki_likely_report
{
  const char *const file;
  const unsigned long line;
  unsigned long counter[2];
};


/*
  Declare section _kroki_likely_report, so that the linker will define
  __start__kroki_likely_report and __stop__kroki_likely_report even when no
  kroki_likely()/kroki_unlikely() is used in the source.
*/
__asm__(".section _kroki_likely_report, \"aw\", @progbits; .previous");


#define _kroki_likely_count(expr, val)                                  \
  __builtin_expect(({                                                   \
        static __attribute__((__section__("_kroki_likely_report")))     \
          struct _kroki_likely_report likely_report =                   \
            { __FILE__, __LINE__, { 0, 0 } };                           \
                                                                        \
        int likely_res = !!(expr);                                      \
                                                                        \
        /* Ignore possible thread race below.  */                       \
        ++likely_report.counter[(likely_res == val)];                   \
                                                                        \
        likely_res;                                                     \
      }), val)

#define kroki_likely(expr)  _kroki_likely_count((expr), 1)
#define kroki_unlikely(expr)  _kroki_likely_count((expr), 0)


/*
  Keep only one copy of _kroki_likely_print_report() and static 'called'
  variable per executable.  Latter is used as a guard to let through
  only the first destructor call from each object file within single
  executable.
*/
__attribute__((__section__(".gnu.linkonce"),
               __visibility__("hidden"),
               __destructor__))
void
_kroki_likely_print_report()
{
  extern __attribute__((__visibility__("hidden")))
    const struct _kroki_likely_report __start__kroki_likely_report,
                                      __stop__kroki_likely_report;

  int count = 0;
  const struct _kroki_likely_report *report;

  static __attribute__((__section__(".gnu.linkonce.b._kroki_likely_report")))
    int called = 0;
  if (called++)
    return;

  fprintf(stderr, "likely()/unlikely() with prediction rate <= %.3f:\n",
          (double) KROKI_LIKELY_REPORT);

  for (report = &__start__kroki_likely_report;
       report != &__stop__kroki_likely_report;
       ++report)
    {
      unsigned long total = report->counter[1] + report->counter[0];
      double ratio;

      if (total == 0)
        continue;

      ratio = (double) report->counter[1] / total;

      if (ratio <= KROKI_LIKELY_REPORT)
        {
          ++count;
          fprintf(stderr, "%s:%lu: +%lu/-%lu (%.3f)\n",
                  report->file, report->line,
                  report->counter[1], report->counter[0],
                  ratio);
        }
    }

  fprintf(stderr, "total %d\n", count);
}


#endif  /* KROKI_LIKELY_REPORT */

#endif  /* ! KROKI_LIKELY_H */
