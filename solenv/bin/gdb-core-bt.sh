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

if command -v gdb >/dev/null
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
    if test -n "$WITH_COREDUMPCTL"; then
        systemd_version=$(systemctl --version | head -1 | awk '{ print $2 }')
        # We need systemd version >= 248 in order for coredumpctl to support --json and --debugger-arguments
        if [ $systemd_version -ge 248 ]
        then
            # Unfortunately `coredumpctl debug` only operates on the most recent core dump matching any
            # given criteria, not on all core dumps matching those criteria; so get the PIDs of all core
            # dumps matching the given COREDUMP_USER_UNIT (and for which a core dump is still present)
            # first, and then iterate over them (though this introduces possibilities for some,
            # hopefully unlikely and mostly harmless, races, like when core dumps disappear in between,
            # or multiple matching core dumps have identical PIDs):
            for i in $($COREDUMPCTL --json=short list COREDUMP_USER_UNIT="$LIBO_TEST_UNIT".scope | \
                           $JQ -r 'map(select(.corefile=="present"))|map(.pid)|join(" ")')
            do
                GDBCOMMANDFILE=$(mktemp)
                printf 'info registers\nthread apply all backtrace full\n' >"$GDBCOMMANDFILE"
                PYTHONWARNINGS=default $COREDUMPCTL debug \
                    COREDUMP_USER_UNIT="$LIBO_TEST_UNIT".scope COREDUMP_PID="$i" \
                    --debugger-arguments="-iex 'add-auto-load-safe-path ${INSTDIR?}' \
                        -x '$GDBCOMMANDFILE' --batch"
                rm "$GDBCOMMANDFILE"
                found=x
            done
        else
            # NoelGrandin - so I can confirm 3 things:
            # (1) virtually none of the jenkins boxes have coredumpctl new enough for the --json option.
            # (2) virtually none of the jenkins boxes have coredumpctl new enough for the --debugger-arguments option
            # (3) Around 50% of the time, coredumpctl will say it has no core files available,
            #     even though we have just dumped core.
            # So just do our best, and hope that we are analysing the right core file.
            #
            sleep 10s
            $COREDUMPCTL -1 list
            # copy core file out of systemd
            TEMPCOREFILE=$(mktemp)
            $COREDUMPCTL -1 -o $TEMPCOREFILE dump
            # analyse core file with gdb
            guess=$(file "$TEMPCOREFILE")
            guess=${guess#* execfn: \'}
            guess=${guess%%\'*}
            if [ ! -x "$guess" ]; then guess=$EXECUTABLE; fi
            printf '\nOne of our tests dumped core, and we are generating a backtrace from the more recent core we can extract from coredumpctl.'
            printf '\n'
            printf '\n>>>>>This might not be the right one.<<<<'
            printf '\n'
            printf '\nIt looks like %s generated %s\nBacktraces:\n' "$guess" "$TEMPCOREFILE"
            GDBCOMMANDFILE=$(mktemp)
            printf 'info registers\nthread apply all backtrace full\n' >"$GDBCOMMANDFILE"
            PYTHONWARNINGS=default gdb -iex "add-auto-load-safe-path ${INSTDIR?}" \
                -x "$GDBCOMMANDFILE" --batch "$guess" "$TEMPCOREFILE"
            rm "$GDBCOMMANDFILE"
            rm "$TEMPCOREFILE"
            found=x
        fi
    fi
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
