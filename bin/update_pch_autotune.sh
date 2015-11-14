#! /bin/bash
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Finds the optimal update_pch settings that results in,
# per module and library, the fastest build time and
# smallest intermediate files (.o/.obj) output.

# Usage: update_pch_autotune.sh [<module1> <module2>]
# Invoke: /opt/lo/bin/make cmd cmd="./bin/update_pch_autotune.sh [..]"

# The resulting values may be entered in update_pch
# to be use for generating PCH in the future.
# Run this script after major header changes.

root=`dirname $0`
root=`cd $root/.. && pwd`
cd $root

if test -z "$1"; then
    modules=`ls ./*/inc/pch/precompiled_*.hxx | sed -e s%./%% -e s%/.*%% | uniq`
else
    modules="$@"
fi

function build()
{
    local START=$(date +%s.%N)

    /opt/lo/bin/make "$module.build" > /dev/null
    status=$?
    if [ $status -ne 0 ];
    then
        # Spurious failures happen.
        /opt/lo/bin/make "$module.build" > /dev/null
        status=$?
    fi

    local END=$(date +%s.%N1)
    build_time=$(printf %.1f $(echo "$END - $START" | bc))

    size="FAILED"
    score="FAILED"
    if [ $status -eq 0 ];
    then
        size="$(du -s workdir/CxxObject/$module/ | awk '{print $1}')"
        score=$(printf %.2f $(echo "10000 / ($build_time * e($size/1048576))" | bc -l))
    fi
}

function run()
{
    local msg="$module.$libname, ${@:3}, "
    printf "$msg"
    ./bin/update_pch "$module" "$libname" "${@:3}" --silent
    status=$?

    if [ $status -eq 0 ];
    then
        build

        summary="$build_time, $size, $score"
        if [ $status -eq 0 ];
        then
            new_best_for_cuttof=$(echo "$score > $best_score_for_cuttof" | bc -l)
            if [ $new_best_for_cuttof -eq 1 ];
            then
                best_score_for_cuttof=$score
            fi

            new_best=$(echo "$score > $best_score" | bc -l)
            if [ $new_best -eq 1 ];
            then
                best_score=$score
                best_args="${@:3}"
                best_time=$build_time
                best_cutoff=$cutoff
                summary="$build_time, $size, $score,*"
            fi
        fi
    else
        # Skip if pch is not updated.
        summary="0, 0, 0"
    fi

    echo "$summary"
}

function args_to_table()
{
    local sys="EXCLUDE"
    local mod="EXCLUDE"
    local loc="EXCLUDE"
    local cutoff=0
    IFS=' ' read -r -a aargs <<< $best_args
    for index in "${!aargs[@]}"
    do
        if [ "${aargs[index]}" = "--include:system" ];
        then
            sys="INCLUDE"
        elif [ "${aargs[index]}" = "--exclude:system" ];
        then
            sys="EXCLUDE"
        elif [ "${aargs[index]}" = "--include:module" ];
        then
            mod="INCLUDE"
        elif [ "${aargs[index]}" = "--exclude:module" ];
        then
            mod="EXCLUDE"
        elif [ "${aargs[index]}" = "--include:local" ];
        then
            loc="INCLUDE"
        elif [ "${aargs[index]}" = "--exclude:local" ];
        then
            loc="EXCLUDE"
        elif [[ "${aargs[index]}" == *"cutoff"* ]]
        then
            cutoff=$(echo "${aargs[index]}" | grep -Po '\-\-cutoff\=\K\d+')
        fi
    done

    local key=$(printf "'%s.%s'" $module $libname)
    echo "$(printf "    %-36s: (%2d, %s, %s, %s), # %5.1f" $key $cutoff $sys $mod $loc $best_time)"
}

for module in $modules; do

    # Build without pch includes as sanity check.
    #run "$root" "$module" --cutoff=999

    # Build before updating pch.
    /opt/lo/bin/make "$module.build" > /dev/null
    if [ $? -ne 0 ];
    then
        # Build with dependencies before updating pch.
        echo "Failed to build $module, building known state with dependencies..."
        ./bin/update_pch.sh "$module" > /dev/null
        /opt/lo/bin/make "$module.clean" > /dev/null
        /opt/lo/bin/make "$module.all" > /dev/null
        if [ $? -ne 0 ];
        then
            # Build all!
            echo "Failed to build $module with dependencies, building all..."
            /opt/lo/bin/make build-nocheck > /dev/null
            if [ $? -ne 0 ];
            then
                >&2 echo "Broken build. Please revert changes and try again."
                exit 1
            fi
        fi
    fi

    # Find pch files in the module to update.
    headers=`find $root/$module/ -type f -iname "precompiled_*.hxx"`

    # Each pch belongs to a library.
    for header in $headers; do
        libname=`echo $header | sed -e s/.*precompiled_// -e s/\.hxx//`
        #TODO: Backup the header and restore when last tune fails.

        # Force update on first try below.
        echo "Autotuning $module.$libname..."
        ./bin/update_pch "$module" "$libname" --cutoff=999 --silent --force

        best_score=0
        best_args=""
        best_time=0
        best_cutoff=0
        for i in {1..16}; do
            cutoff=$i
            best_score_for_cuttof=0
            #run "$root" "$module" "--cutoff=$i" --include:system --exclude:module --exclude:local
            run "$root" "$module" "--cutoff=$i" --exclude:system --exclude:module --exclude:local
            #run "$root" "$module" "--cutoff=$i" --include:system --include:module --exclude:local
            run "$root" "$module" "--cutoff=$i" --exclude:system --include:module --exclude:local
            #run "$root" "$module" "--cutoff=$i" --include:system --exclude:module --include:local
            run "$root" "$module" "--cutoff=$i" --exclude:system --exclude:module --include:local
            #run "$root" "$module" "--cutoff=$i" --include:system --include:module --include:local
            run "$root" "$module" "--cutoff=$i" --exclude:system --include:module --include:local

            if [ $i -gt $((best_cutoff+2)) ];
            then
                score_too_low=$(echo "$best_score_for_cuttof < $best_score / 1.10" | bc -l)
                if [ $score_too_low -eq 1 ];
                then
                    echo "Score hit low of $best_score_for_cuttof, well bellow overall best of $best_score. Stopping."
                    break;
                fi
            fi
        done

        ./bin/update_pch "$module" "$libname" $best_args --force --silent
        echo "> $module.$libname, $best_args, $best_time, $size, $score"
        echo

        table+=$'\n'
        table+="$(args_to_table)"
    done

done

echo "Update the relevant lines in ./bin/update_pch script:"
>&2 echo "$table"

exit 0
