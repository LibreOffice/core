dnl -*- Mode: Autoconf; tab-width: 4; indent-tabs-mode: nil; fill-column: 102 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

m4_define([_libo_define_pkg_version], [
    save_IFS="$IFS"
    IFS=.
    echo "$ver" | while read major minor micro; do
        AC_DEFINE_UNQUOTED([$1_VERSION_MAJOR], [$major])
        AC_DEFINE_UNQUOTED([$1_VERSION_MINOR], [$minor])
        AC_DEFINE_UNQUOTED([$1_VERSION_MICRO], [$micro])
    done
    IFS="$save_IFS"
])

m4_define([_libo_define_pkg_version_direct], [
    AC_DEFINE([$1_VERSION_MAJOR], [$2])
    AC_DEFINE([$1_VERSION_MINOR], [$3])
    AC_DEFINE([$1_VERSION_MICRO], [$4])
])

# libo_PKG_VERSION(VARIABLE-STEM, MODULE, BUNDLED-VERSION)
AC_DEFUN([libo_PKG_VERSION], [
    AS_IF([test -n "$SYSTEM_$1"], [
        AC_REQUIRE([PKG_PROG_PKG_CONFIG])
        AC_MSG_CHECKING([for $1 version])
        AS_IF([test -n "$PKG_CONFIG"], [
            ver=`$PKG_CONFIG --modversion "$2"`
            AS_IF([test -n "$ver"], [
                AC_MSG_RESULT([$ver])
                _libo_define_pkg_version([$1], [$ver])
            ], [
                AC_MSG_ERROR([not found])
            ])
        ], [
            AC_MSG_ERROR([not found])
        ])
    ], [
        _libo_define_pkg_version_direct([$1], m4_translit([$3], [.], [,]))
    ])
])

dnl vim:set shiftwidth=4 softtabstop=4 expandtab:
