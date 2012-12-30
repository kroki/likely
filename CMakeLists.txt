cmake_minimum_required(VERSION 2.6)
project(kroki-likely C)
install(FILES likely.h DESTINATION include/kroki)

set(CPACK_GENERATOR RPM)

add_custom_target(version ALL
    ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_SOURCE_DIR}/version.cmake")

set(CPACK_INSTALL_CMAKE_PROJECTS
    "${CMAKE_BINARY_DIR};${CMAKE_PROJECT_NAME};ALL;/")
set(CPACK_PACKAGE_NAME "${CMAKE_PROJECT_NAME}-devel")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "Define likely() and unlikely() macros to guide branch prediction.")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")

set(CPACK_RPM_PACKAGE_ARCHITECTURE noarch)
set(CPACK_RPM_PACKAGE_LICENSE LGPLv3+)
set(CPACK_RPM_PACKAGE_GROUP Development/Libraries)
set(CPACK_RPM_PACKAGE_URL "https://github.com/kroki/likely")

set(CPACK_PROJECT_CONFIG_FILE "${CMAKE_BINARY_DIR}/version.gen")

include(CPack)
