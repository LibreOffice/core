dnl -*- Mode: Autoconf; tab-width: 4; indent-tabs-mode: nil; fill-column: 102 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# <lowercase check>,<variable prefix>,<pkg-config query>,
#     <internal CFLAGS>,<internal LIBS>,<external (default: FALSE)>
AC_DEFUN([libo_CHECK_SYSTEM_MODULE], [
AC_ARG_WITH(system-$1,
    AS_HELP_STRING([m4_ifnblank([$6],[--without-system-$1],[--with-system-$1])],
        [m4_ifnblank([$6],
            Build and bundle the internal $1 instead of using the operating system one.,
            Use $1 from the operating system instead of building and bundling it.)]),
,[m4_ifnblank([$6],[with_system_$1="yes"],[with_system_$1="$with_system_libs"])])
AC_MSG_CHECKING([which $1 to use])
if test "$test_$1" != "no"; then
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
    AC_MSG_RESULT([ignored])
fi
AC_SUBST([ENABLE_$2])
AC_SUBST([SYSTEM_$2])
AC_SUBST([$2_CFLAGS])
AC_SUBST([$2_LIBS])
])

dnl vim:set shiftwidth=4 softtabstop=4 expandtab:
