#!/usr/bin/env bash

SCRIPT=$(realpath "$0")
SCRIPTPATH=$(dirname "$SCRIPT")
PATHS="$(find $SCRIPTPATH/.. \( -wholename '*/qa/*/testdocuments' -o -wholename '*/qa/*/testdocuments/*' -o -wholename '*/qa/*/data' -o -wholename '*/qa/*/data/*' \) -type d )"

for path in $PATHS
do
    # Ignore pass/fail/indeterminate folders, functions test in sc, workdir folder and xml in sd
    if [[ "$path" != */pass* ]] && [[ "$path" != */fail* ]] && [[ "$path" != */indeterminate* ]] \
            && [[ "$path" != */functions* ]] && [[ "$path" != */workdir* ]] && [[ "$path" != */xml* ]]; then
        for i in $path/*
        do
            if [ -f "$i" ]; then
                file=$(basename "$i")
                if ! git grep -q "$file"; then
                    echo "WARNING: $i is not used, write a testcase for it!"
                fi
            fi
        done
    fi
done

# vi:set shiftwidth=4 expandtab:
