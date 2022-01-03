dnl -*- Mode: Autoconf; tab-width: 4; indent-tabs-mode: nil; fill-column: 102 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
AC_DEFUN([libo_ENABLE_VCLPLUG],[
dnl The $1 of enable_var is taken from the libo_ENABLE_VCLPLUG, so concat
dnl needs extra quoting and enable_var has no "magic" argument.
m4_pushdef([concat],[$][1][$][2])
m4_pushdef([enable_var],[concat(ENABLE_,m4_toupper($1))])

enable_var=
if test "$test_$1" != no -a "$enable_$1" = yes; then
    enable_var=TRUE
    AC_DEFINE(enable_var)
    R="$R $1"
fi
AC_SUBST(enable_var)

m4_popdef([enable_var])
m4_popdef([concat])
])

dnl vim:set shiftwidth=4 softtabstop=4 expandtab:
