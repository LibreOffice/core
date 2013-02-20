# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
AC_DEFUN([libo_PUBLISH_MODULE],
[
    true
    AC_SUBST([SYSTEM_$1])
    AC_SUBST([$1_CFLAGS])
    AC_SUBST([$1_LIBS])
])

AC_DEFUN([libo_CHECK_SYSTEM_MODULE], [
AC_MSG_CHECKING([which $1 to use])
if test "$with_system_$1" = "yes"; then
    AC_MSG_RESULT([external])
    SYSTEM_$2=YES
    PKG_CHECK_MODULES([$2], [$3])
else
    AC_MSG_RESULT([internal])
    SYSTEM_$2=NO
    $2_CFLAGS=$4
    $2_LIBS=$5
    BUILD_TYPE="$BUILD_TYPE $2"
fi
    libo_PUBLISH_MODULE([$2])
])
# vim: set noet sw=4 ts=4:
