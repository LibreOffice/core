dnl -*- Mode: Autoconf; tab-width: 4; indent-tabs-mode: nil; fill-column: 102 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# <lowercase check>,<variable prefix>,<pkg-config query>,
#     <internal CFLAGS>,<internal LIBS>,
#     <can be toggled: enabled, disabled or fixed (default: fixed)>,
#     <prefer: system, fixed-system, internal or fixed-internal (default: internal)>
#
# fixed == fixed-enabled, as fixed-disabled makes no sense.
#
# Used configure.ac variables:
#  - enable_$1: should normally not be set manually; use test_$1 instead
#  - found_$1: other tests already provided external $2_CFLAGS and $2_LIBS
#  - test_$1: set to no, if the feature shouldn't be tested at all
#  - test_system_$1: set to no, if the system library should not be used
#

m4_define([csm_fatal_error], [
    errprint(__program__:__file__:__line__`: fatal error: $*
')
    m4_exit([1])
])

m4_define([csm_default_with], [
    if test "${with_system_$1+set}" != set -a "${with_system_libs+set}" = set; then
        with_system_$1="$with_system_libs";
    else
         with_system_$1="$2"
    fi
])

AC_DEFUN([libo_CHECK_SYSTEM_MODULE], [
m4_if(
    [$6],[enabled],[
        AC_ARG_ENABLE([$1],
            AS_HELP_STRING([--disable-$1],[Disable $1 support.]),
        ,[enable_$1="yes"])
    ],[$6],[disabled],[
        AC_ARG_ENABLE([$1],
            AS_HELP_STRING([--enable-$1],[Enable $1 support.]),
        ,[enable_$1="no"])
    ],[
        m4_if([$6],[fixed],[],[m4_ifnblank([$6],
              [csm_fatal_error(`"$6" must be "enabled", "disabled", "fixed" or empty (=fixed)')])])
        enable_$1="yes";
])
m4_if(
    [$7],[system],[
        AC_ARG_WITH(system-$1,
            AS_HELP_STRING([--without-system-$1],[Build and bundle the internal $1.]),
        ,[csm_default_with($1,yes)])
    ],[$7],[fixed-system],[
        with_system_$1=yes
    ],[$7],[fixed-internal],[
        with_system_$1=no
    ],[
        m4_if([$7],[internal],[],[m4_ifnblank([$7],
              [csm_fatal_error(`"$7" must be "(fixed-)system", "(fixed-)internal" or empty (=internal)')])])
        AC_ARG_WITH(system-$1,
            AS_HELP_STRING([--with-system-$1],[Use $1 from the operating system.]),
        ,[csm_default_with($1,no)])
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
        $2_CFLAGS=$4
        $2_LIBS=$5
        BUILD_TYPE="$BUILD_TYPE $2"
    fi
else
    if test "$found_$1" = yes -a "$enable_$1" != no -a "$with_system_$1" = yes; then
        AC_MSG_RESULT([external])
        ENABLE_$2=TRUE
        SYSTEM_$2=TRUE
    elif test "$test_$1" != no -a "$enable_$1" = no; then
        AC_MSG_RESULT([disabled])
    else
        AC_MSG_RESULT([ignored / not supported by OS])
    fi
fi
AC_SUBST([ENABLE_$2])
AC_SUBST([SYSTEM_$2])
AC_SUBST([$2_CFLAGS])
AC_SUBST([$2_LIBS])
])

dnl vim:set shiftwidth=4 softtabstop=4 expandtab:
