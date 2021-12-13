dnl -*- Mode: Autoconf; tab-width: 4; indent-tabs-mode: nil; fill-column: 102 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# <lowercase check>,<variable prefix>,<pkg-config query>,
#     <internal CFLAGS>,<internal LIBS>,<prefer external (default: FALSE)>,
#     <can be disabled (default: FALSE)>
#
# FALSE is actually a blank value, so TRUE matches any not-blank value.
#
# Used configure.ac variables:
#  - enable_$1: should normally not be set manually; use test_$1 instead
#  - test_$1: set to no, if the feature shouldn't be tested at all
#  - test_system_$1: set to no, if the system library should not be used
#
AC_DEFUN([libo_CHECK_SYSTEM_MODULE], [
AC_ARG_WITH(system-$1,
    AS_HELP_STRING([m4_ifnblank([$6],[--without-system-$1],[--with-system-$1])],
        [m4_ifnblank([$6],
            Build and bundle the internal $1 instead of using the operating system one.,
            Use $1 from the operating system instead of building and bundling it.)]),
,[m4_ifnblank([$6],[with_system_$1="yes"],[with_system_$1="$with_system_libs"])])
m4_ifnblank([$7],[
    AC_ARG_ENABLE([$1],
        AS_HELP_STRING([--disable-$1],[Disable $1 support.]),
    ,[enable_$1=yes])
])
AC_MSG_CHECKING([which $1 to use])
if test "$test_$1" != no -a "$enable_$1" != no; then
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
    if test "$test_$1" != no -a "$enable_$1" = no; then
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
