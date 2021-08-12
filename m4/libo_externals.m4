dnl -*- Mode: Autoconf; tab-width: 4; indent-tabs-mode: nil; fill-column: 102 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# <$1 lowercase variable part - used for variables and configure switches>
# <$2 uppercase variable part - used for configure.ac and make variables>
# <$3 pkg-config query string>
# [$4 if optional, default to: enabled, disabled or fixed (default: fixed)]
# [$5 which is preferred: (fixed-|test-)system or (fixed-)internal or system-if-linux (default: internal)]
# [$6 ignore $with_system_libs: TRUE or blank (default: blank/false)]
#
# $4 fixed: fixed-enabled, as fixed-disabled makes no sense.
# $5 test-system: follows $test_system_$1, ignores $with_system_libs; no configure switch
#
# Used configure.ac variables:
#  - $2_(CFLAGS|LIBS)_internal: must be filled to match the internal build
#  - enable_$1: should normally not be set manually; use test_$1 instead
#  - found_$1: other tests already provided external $2_CFLAGS and $2_LIBS
#  - test_$1: set to no, if the feature shouldn't be tested at all
#  - test_system_$1: set to no, if the system library should not be used
#
# There is currently the AC_SUBST redundancy of
#   (SYSTEM_$2,TRUE) == (,$(filter $2,$(BUILD_TYPE)))
#

m4_define([csm_default_with], [
    if test "${with_system_$1+set}" != set -a "${with_system_libs+set}" = set -a "$3" != TRUE; then
        with_system_$1="$with_system_libs";
    else
        with_system_$1="$2"
    fi
])

m4_define([csm_check_required], [
    m4_ifblank([$2],[m4_fatal([$][$1 ($2) must not be blank and $4])])
    m4_if([$2],[$3],,[m4_fatal([$][$1 ($2) $3 must be $4])])
])

AC_DEFUN([libo_CHECK_SYSTEM_MODULE], [
# validate arguments as possible
csm_check_required([1],[$1],m4_tolower([$1]),[lowercase])
csm_check_required([2],[$2],m4_toupper([$2]),[uppercase])
m4_ifblank([$3],[m4_fatal([$][3 is the pkg-config query and must not be blank])])
m4_if([$6],[TRUE],[],[m4_ifnblank([$6],[m4_fatal([$][6 must be TRUE or blank])])])
m4_if(
    [$4],[enabled],[
        AC_ARG_ENABLE([$1],
            AS_HELP_STRING([--disable-$1],[Disable $1 support.]),
        ,[enable_$1="yes"])
    ],[$4],[disabled],[
        AC_ARG_ENABLE([$1],
            AS_HELP_STRING([--enable-$1],[Enable $1 support.]),
        ,[enable_$1="no"])
    ],[
        m4_if([$4],[fixed],,[m4_ifnblank([$4],
              [m4_fatal([$$4 ($4) must be "enabled", "disabled", "fixed" or empty (=fixed)])])])
        enable_$1="yes";
])
m4_if(
    [$5],[system],[
        AC_ARG_WITH(system-$1,
            AS_HELP_STRING([--without-system-$1],[Build and bundle the internal $1.]),
        ,[csm_default_with($1,yes,$6)])
    ],[$5],[test-system],[
        with_system_$1="$test_system_$1"
    ],[$5],[fixed-system],[
        with_system_$1=yes
    ],[$5],[fixed-internal],[
        with_system_$1=no
    ],[$5],[system-if-linux],[
        AC_ARG_WITH(system-$1,
            AS_HELP_STRING([--with-system-$1],[Use $1 from the operating system.]),
        ,[case "$_os" in
            Linux)
                with_system_nss=yes
            ;;
            *)
                with_system_nss=no
            ;;
          esac])
    ],[
        m4_if([$5],[internal],,[m4_ifnblank([$5],
              [m4_fatal([$$5 ($5) must be "(fixed-|test-)system", "(fixed-)internal", "system-if-linux" or empty (=internal)])])])
        AC_ARG_WITH(system-$1,
            AS_HELP_STRING([--with-system-$1],[Use $1 from the operating system.]),
        ,[csm_default_with($1,no,$6)])
])

AC_MSG_CHECKING([which $1 to use])
if test "$test_$1" != no -a "$found_$1" != yes -a "$enable_$1" != no; then
    ENABLE_$2=TRUE
    if test "$with_system_$1" = yes -a "$test_system_$1" != no; then
        AC_MSG_RESULT([external])
        SYSTEM_$2=TRUE
        PKG_CHECK_MODULES([$2], [$3])
        $2_CFLAGS=$(printf '%s' "${$2_CFLAGS}" | sed -e "s/-I/${ISYSTEM?}/g")
        FilterLibs "${$2_LIBS}"
        $2_LIBS="$filteredlibs"
    else
        AC_MSG_RESULT([internal])
        SYSTEM_$2=
        $2_CFLAGS="${$2_CFLAGS_internal}"
        $2_LIBS="${$2_LIBS_internal}"
        BUILD_TYPE="$BUILD_TYPE $2"
    fi
else
    if test "$found_$1" = yes -a "$enable_$1" != no -a "$with_system_$1" = yes; then
        AC_MSG_RESULT([external])
        ENABLE_$2=TRUE
        SYSTEM_$2=TRUE
    else
        ENABLE_$2=
        SYSTEM_$2=
        $2_CFLAGS=
        $2_LIBS=
        if test "$test_$1" != no -a "$enable_$1" = no; then
            AC_MSG_RESULT([disabled])
        else
            AC_MSG_RESULT([not tested])
        fi
    fi
fi
AC_SUBST([ENABLE_$2])
AC_SUBST([SYSTEM_$2])
AC_SUBST([$2_CFLAGS])
AC_SUBST([$2_LIBS])
])

dnl vim:set shiftwidth=4 softtabstop=4 expandtab:
