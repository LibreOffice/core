# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
AC_DEFUN([libo_CHECK_EXTENSION],[
AC_MSG_CHECKING([for $1 extension integration])
$2_EXTENSION_PACK=
if test "x$enable_ext_$3" = "xyes" -a "x$enable_extension_integration" != "xno"; then
    SCPDEFS="$SCPDEFS -DWITH_EXTENSION_$2"
    $2_EXTENSION_PACK="$4"
    BUILD_TYPE="$BUILD_TYPE $2"
    WITH_EXTRA_EXTENSIONS=YES
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
fi
AC_SUBST($2_EXTENSION_PACK)
])
# vim: set noet sw=4 ts=4:
