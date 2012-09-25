#!/bin/sh
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

#
# STAR_PROFILE_LOCKING_DISABLED=1
# export STAR_PROFILE_LOCKING_DISABLED
#

# file locking now enabled by default
SAL_ENABLE_FILE_LOCKING=1
export SAL_ENABLE_FILE_LOCKING

# Uncomment the line below if you suspect that OpenGL is not
# working on your system.
# SAL_NOOPENGL=true; export SAL_NOOPENGL

# uncomment line below to disable anti aliasing of fonts
# SAL_ANTIALIAS_DISABLE=true; export SAL_ANTIALIAS_DISABLE

# uncomment line below if you encounter problems starting soffice on your system
# SAL_NO_XINITTHREADS=true; export SAL_NO_XINITTHREADS

#@JITC_PROCESSOR_TYPE_EXPORT@

# resolve installation directory
sd_cwd=`pwd`
sd_res=$0
while [ -h "$sd_res" ] ; do
    cd "`dirname "$sd_res"`"
    sd_basename=`basename "$sd_res"`
    sd_res=`ls -l "$sd_basename" | sed "s/.*$sd_basename -> //g"`
done
cd "`dirname "$sd_res"`"
sd_prog=`pwd`
cd "$sd_cwd"

# linked build needs additional settings
if [ -e $sd_prog/ooenv ] ; then
    . $sd_prog/ooenv
fi

# try to get some debug output?
GDBTRACECHECK=
STRACECHECK=
VALGRINDCHECK=

# count number of selected checks; only one is allowed
checks=
# force the --valgrind option if the VALGRIND variable is set
test -n "$VALGRIND" && VALGRINDOPT="--valgrind" || VALGRINDOPT=

for arg in $@ $VALGRINDOPT ; do
    case "$arg" in
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
                valgrind_ver=`valgrind --version | sed -e "s/valgrind-//"`
                valgrind_ver_maj=`echo $valgrind_ver | awk -F. '{ print \$1 }'`
                valgrind_ver_min=`echo $valgrind_ver | awk -F. '{ print \$2 }'`
                valgrind_skip=
                if [ "$valgrind_ver_maj" -gt 3 -o \( "$valgrind_ver_maj" -eq 3 -a "$valgrind_ver_min" -ge 6 \) ] ; then
                    valgrind_skip='--trace-children-skip=*/java'
                fi
                # finally set the valgrind check
                VALGRINDCHECK="valgrind --tool=$VALGRIND --trace-children=yes $valgrind_skip --num-callers=50 --error-limit=no"
                echo "use kill -SIGUSR2 pid to dump traces of active allocations"
                checks="c$checks"
                if [ "$VALGRIND" = "memcheck" ] ; then
                    export G_SLICE=always-malloc
                    export GLIBCXX_FORCE_NEW=1
                fi
            else
                echo "Error: Can't find the tool \"valgrind\", --valgrind option will be ignored"
                exit 1
            fi
            ;;
    esac
done

if echo "$checks" | grep -q "cc" ; then
    echo "Error: The debug options --backtrace, --strace, and --valgrind cannot be used together."
    echo "       Please, use them one by one."
    exit 1;
fi

case "`uname -s`" in
NetBSD|OpenBSD|FreeBSD|DragonFly)
# this is a temporary hack until we can live with the default search paths
    sd_prog1="$sd_prog"
    sd_prog2="$sd_prog/../ure-link/lib"
    LD_LIBRARY_PATH=$sd_prog1:$sd_prog2${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
    JAVA_HOME=$(javaPathHelper -h libreoffice-java 2> /dev/null)
    export LD_LIBRARY_PATH
    if [ -n "${JAVA_HOME}" ]; then
        export JAVA_HOME
    fi
    ;;
AIX)
    LIBPATH=$sd_prog:$sd_prog/../ure-link/lib${LIBPATH:+:$LIBPATH}
    export LIBPATH
    ;;
esac

# run soffice.bin directly when you want to get the backtrace
if [ -n "$GDBTRACECHECK" ] ; then
    exec $GDBTRACECHECK "$sd_prog/soffice.bin" "$@"
fi

# valgrind --log-file=valgrind.log does not work well with --trace-children=yes
if [ -n "$VALGRINDCHECK" -a -z "$VALGRIND" ] ; then
    echo "redirecting the standard and the error output to valgrind.log"
    exec &>valgrind.log
fi

# oosplash does the rest: forcing pages in, javaldx etc. are
exec $VALGRINDCHECK $STRACECHECK "$sd_prog/oosplash" "$@"
