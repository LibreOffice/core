#!/bin/sh
#
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
#       Bjoern Michaelsen <bjoern.michaelsen@canonical.com>
# Portions created by the Initial Developer are Copyright (C) 2010 Canonical,
# Ltd. . All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.
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
        echo "thread apply all backtrace" > "$GDBCOMMANDFILE"
        gdb -x "$GDBCOMMANDFILE" --batch "$EXECUTABLE" "$COREFILE"
        rm "$GDBCOMMANDFILE"
        echo
        exit 0
    elif [ $EXITCODE -ge 128 ]; then
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
