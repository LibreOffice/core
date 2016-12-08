dnl -*- Mode: Autoconf; tab-width: 4; indent-tabs-mode: nil; fill-column: 102 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
AC_DEFUN([libo_FUZZ_ARG_WITH], [
    AC_ARG_WITH([$1],
        [$2],
        [$3],
        if test "$enable_fuzz_options" = yes; then
            if test `expr $RANDOM % 2` = 1; then
                m4_translit([with-$1], [-+.], [___])=yes
            else
                m4_translit([with-$1], [-+.], [___])=no
            fi
            AC_MSG_NOTICE([Randomly set --with-$1=$m4_translit([with-$1], [-+.], [___])])
            libo_fuzzed_[]m4_translit([with-$1], [-+.], [___])=yes
            libo_fuzz_list="$libo_fuzz_list --with-$1="'$m4_translit([with-$1], [-+.], [___])'
        fi
        [$4]
        )
])

AC_DEFUN([libo_FUZZ_ARG_ENABLE], [
    AC_ARG_ENABLE([$1],
        [$2],
        [$3],
        if test "$enable_fuzz_options" = yes; then
            if test `expr $RANDOM % 2` = 1; then
                m4_translit([enable-$1], [-+.], [___])=yes
            else
                m4_translit([enable-$1], [-+.], [___])=no
            fi
            AC_MSG_NOTICE([Randomly set --enable-$1=$m4_translit([enable-$1], [-+.], [___])])
            libo_fuzzed_[]m4_translit([enable-$1], [-+.], [___])=yes
            libo_fuzz_list="$libo_fuzz_list --enable-$1="'$m4_translit([enable-$1], [-+.], [___])'
        fi
        [$4])
])

AC_DEFUN([libo_FUZZ_SUMMARY], [
    if test -n "$libo_fuzz_list"; then
        tmps=`eval echo $libo_fuzz_list`
        AC_MSG_NOTICE([Summary of fuzzing: $tmps])
    fi
])

dnl vim:set shiftwidth=4 softtabstop=4 expandtab:
