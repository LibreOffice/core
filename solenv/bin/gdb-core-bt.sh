#!/bin/sh
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#

EXECUTABLE=${1}
COREDIR=${2}
EXITCODE=${3}

if test -n "$(which gdb)"
then
    found=
    for COREFILE in "$COREDIR"/core*
    do
        if [ -f "$COREFILE" ]
        then
            guess=$(file "$COREFILE")
            guess=${guess#* execfn: \'}
            guess=${guess%%\'*}
            if [ ! -x "$guess" ]; then guess=$EXECUTABLE; fi
            printf '\nIt looks like %s generated %s\nBacktraces:\n' \
                "$guess" "$COREFILE"
            GDBCOMMANDFILE=$(mktemp)
            printf "info registers\nthread apply all backtrace full\n" \
                >"$GDBCOMMANDFILE"
            PYTHONWARNINGS=default gdb -iex "add-auto-load-safe-path ${INSTDIR?}" \
                -x "$GDBCOMMANDFILE" --batch "$guess" "$COREFILE" && found=x
            rm "$GDBCOMMANDFILE"
            echo
        fi
    done
    if [ -z "$found" -a "$EXITCODE" -ge 128 ]; then
        echo
        echo "No core file identified in directory ${COREDIR}"
        echo "To show backtraces for crashes during test execution,"
        echo "enable core files with:"
        echo
        echo "   ulimit -c unlimited"
        echo
        exit 1
    fi
else
    echo "You need gdb in your path to show backtraces"
    exit 1
fi
