dnl -*- Mode: Autoconf; tab-width: 4; indent-tabs-mode: nil; fill-column: 102 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
AC_DEFUN([libo_CHECK_SYSTEM_MODULE], [
AC_ARG_WITH(system-$1,
    AS_HELP_STRING([--with-system-$1],
        [Use $1 from operating system instead of building and bundling it.]),,
    [with_system_$1="$with_system_libs"])
AC_MSG_CHECKING([which $1 to use])
if test "$with_system_$1" = "yes"; then
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
AC_SUBST([SYSTEM_$2])
AC_SUBST([$2_CFLAGS])
AC_SUBST([$2_LIBS])
])

dnl vim:set shiftwidth=4 softtabstop=4 expandtab:
