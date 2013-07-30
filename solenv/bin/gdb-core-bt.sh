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
    if test "$(ls "$COREDIR"/core* 2>/dev/null | wc -l)" -eq 1
    then
        COREFILE=$(ls "$COREDIR"/core*)
        echo
        echo "It looks like ${EXECUTABLE} generated a core file at ${COREFILE}"
        echo "Backtraces:"
        GDBCOMMANDFILE=`mktemp`
        echo "thread apply all backtrace full" > "$GDBCOMMANDFILE"
        gdb -x "$GDBCOMMANDFILE" --batch "$EXECUTABLE" "$COREFILE"
        rm "$GDBCOMMANDFILE"
        echo
        exit 0
    elif [ "$EXITCODE" -ge 128 ]; then
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
