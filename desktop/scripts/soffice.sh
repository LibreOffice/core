#!/bin/sh
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

# use POSIX locale for well-defined tool output
LO_SAVE_LC_ALL="$LC_ALL"
LC_ALL=C
export LC_ALL

#
# STAR_PROFILE_LOCKING_DISABLED=1
# export STAR_PROFILE_LOCKING_DISABLED
#

# file locking now enabled by default
SAL_ENABLE_FILE_LOCKING=1
export SAL_ENABLE_FILE_LOCKING

# uncomment line below to disable anti aliasing of fonts
# SAL_ANTIALIAS_DISABLE=true; export SAL_ANTIALIAS_DISABLE

# uncomment line below if you encounter problems starting soffice on your system
# SAL_NO_XINITTHREADS=true; export SAL_NO_XINITTHREADS

#@JITC_PROCESSOR_TYPE_EXPORT@

# resolve installation directory
sd_cwd=$(pwd)
sd_res="$0"
while [ -h "$sd_res" ] ; do
    sd_dirname=$(dirname "$sd_res")
    cd "$sd_dirname" || exit $?
    sd_basename=$(basename "$sd_res")
    sd_res=$(ls -l "$sd_basename" | sed "s/.*$sd_basename -> //g")
done
sd_dirname=$(dirname "$sd_res")
cd "$sd_dirname" || exit $?
sd_prog=$(pwd)
cd "$sd_cwd" || exit $?

# linked build needs additional settings
if [ -e "${sd_prog}/ooenv" ] ; then
    # shellcheck source=../../instsetoo_native/ooenv
    . "${sd_prog}/ooenv"
fi

# try to get some debug output?
GDBTRACECHECK=
STRACECHECK=
VALGRINDCHECK=
RRCHECK=

# count number of selected checks; only one is allowed
checks=
EXTRAOPT=
# force the --valgrind option if the VALGRIND variable is set
test -n "$VALGRIND" && EXTRAOPT="--valgrind"

# force the --record option if the RR variable is set
test -n "$RR" && EXTRAOPT="--record"

for arg in "$@" $EXTRAOPT ; do
    case "$arg" in
        --record)
            if which rr >/dev/null 2>&1 ; then
                # smoketest may already be recorded => ignore nested
                RRCHECK="rr record --nested=ignore"
                checks="c$checks"
            else
                echo "Error: Can't find the tool \"rr\", --record option will be ignored."
                exit 1
            fi
            ;;
        --backtrace)
            if which gdb >/dev/null 2>&1 ; then
                GDBTRACECHECK="gdb -nx --command=$sd_prog/gdbtrace --args"
                checks="c$checks"
            else
                echo "Error: Can't find the tool \"gdb\", --backtrace option will be ignored."
                exit 1
            fi
            ;;
        --strace)
            if which strace >/dev/null 2>&1 ; then
                STRACECHECK="strace -o strace.log -f -tt -s 256"
                checks="c$checks"
            else
                echo "Error: Can't find the tool \"strace\", --strace option will be ignored."
                exit 1;
            fi
            ;;
         --valgrind)
            test -n "$VALGRINDCHECK" && continue;
            if which valgrind >/dev/null 2>&1 ; then
                # another valgrind tool might be forced via the environment variable
                test -z "$VALGRIND" && VALGRIND="memcheck"
                # --trace-children-skip is pretty useful but supported only with valgrind >= 3.6.0
                valgrind_ver=$(valgrind --version | sed -e "s/valgrind-//")
                valgrind_ver_maj=$(echo "$valgrind_ver" | awk -F. '{ print $1 }')
                valgrind_ver_min=$(echo "$valgrind_ver" | awk -F. '{ print $2 }')
                valgrind_skip=
                if [ "$valgrind_ver_maj" -gt 3 ] || ( [ "$valgrind_ver_maj" -eq 3 ] && [ "$valgrind_ver_min" -ge 6 ] ) ; then
                    valgrind_skip='--trace-children-skip=*/java,*/gij'
                fi
                # finally set the valgrind check
                VALGRINDCHECK="valgrind --tool=$VALGRIND --trace-children=yes $valgrind_skip --num-callers=50 --error-limit=no"
                echo "use kill -SIGUSR2 pid to dump traces of active allocations"
                checks="c$checks"
                case "$VALGRIND" in
                helgrind|memcheck|massif|exp-dhat)
                    export G_SLICE=always-malloc
                    export GLIBCXX_FORCE_NEW=1
                    ;;
                callgrind)
                    unset MALLOC_CHECK_ MALLOC_PERTURB_ G_SLICE
                    export SAL_DISABLE_FLOATGRAB=1
                    export OOO_DISABLE_RECOVERY=1
                    export SAL_DISABLE_WATCHDOG=1
                    export LD_BIND_NOW=1
                    ;;
                esac
            else
                echo "Error: Can't find the tool \"valgrind\", --valgrind option will be ignored"
                exit 1
            fi
            ;;
    esac
done

if echo "$checks" | grep -q "cc" ; then
    echo "Error: The debug options --record, --backtrace, --strace, and --valgrind cannot be used together."
    echo "       Please, use them one by one."
    exit 1;
fi

case "$(uname -s)" in
OpenBSD)
# this is a temporary hack until we can live with the default search paths
    LD_LIBRARY_PATH="$sd_prog${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
    JAVA_HOME=$(javaPathHelper -h libreoffice-java 2> /dev/null)
    export LD_LIBRARY_PATH
    if [ -n "${JAVA_HOME}" ]; then
        export JAVA_HOME
    fi
    ;;
NetBSD|DragonFly)
# this is a temporary hack until we can live with the default search paths
    LD_LIBRARY_PATH="$sd_prog${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
    export LD_LIBRARY_PATH
    ;;
AIX)
    LIBPATH="$sd_prog${LIBPATH:+:$LIBPATH}"
    export LIBPATH
    ;;
esac

# restore locale setting, avoiding to export empty LC_ALL, s. tdf#130080
if [ -n "$LO_SAVE_LC_ALL" ]; then
    LC_ALL="$LO_SAVE_LC_ALL"
else
    unset LC_ALL
fi

test "${STATIC_UNO_HOME+set}" = set || export STATIC_UNO_HOME="file://${sd_prog}"

# run soffice.bin directly when you want to get the backtrace
if [ -n "$GDBTRACECHECK" ] ; then
    exec $GDBTRACECHECK "$sd_prog/soffice.bin" "$@"
fi

# valgrind --log-file=valgrind.log does not work well with --trace-children=yes
if [ -n "$VALGRINDCHECK" ] && [ -z "$VALGRIND" ] ; then
    echo "redirecting the standard and the error output to valgrind.log"
    exec > valgrind.log 2>&1
fi

# oosplash does the rest: forcing pages in, javaldx etc. are
exec $RRCHECK $VALGRINDCHECK $STRACECHECK "$sd_prog/oosplash" "$@"
