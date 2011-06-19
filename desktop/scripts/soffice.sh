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

unset XENVIRONMENT

# uncomment line below to disable anti aliasing of fonts
# SAL_ANTIALIAS_DISABLE=true; export SAL_ANTIALIAS_DISABLE

# uncomment line below if you encounter problems starting soffice on your system
# SAL_NO_XINITTHREADS=true; export SAL_NO_XINITTHREADS

# read database entries for Adabas D
if [ -f /etc/adabasrc ]; then
  . /etc/adabasrc
fi

# The following is needed on Linux PPC with IBM j2sdk142:
#@# export JITC_PROCESSOR_TYPE=6

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

if [ "$VALGRIND" != "" ]; then
    VALGRINDCHECK="valgrind --tool=$VALGRIND --trace-children=yes --trace-children-skip=*/java --error-exitcode=101"
    export VALGRINDCHECK
    if [ "$VALGRIND" = "memcheck" ]; then
        G_SLICE=always-malloc
        export G_SLICE
    fi
fi

case "`uname -s`" in
NetBSD|OpenBSD|FreeBSD|DragonFly)
# this is a temporary hack until we can live with the default search paths
    sd_prog1="$sd_prog/../basis-link/program"
    sd_prog2="$sd_prog/../basis-link/ure-link/lib"
    LD_LIBRARY_PATH=$sd_prog1:$sd_prog2${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
    JAVA_HOME=$(javaPathHelper -h libreoffice-java 2> /dev/null)
    export LD_LIBRARY_PATH
    if [ -n "${JAVA_HOME}" ]; then
        export JAVA_HOME
    fi
    ;;
AIX)
    LIBPATH=$sd_prog:$sd_prog/../basis-link/program:$sd_prog/../basis-link/ure-link/lib${LIBPATH:+:$LIBPATH}
    export LIBPATH
    ;;
esac

# oosplash does the rest: forcing pages in, javaldx etc. are
exec $VALGRINDCHECK "$sd_prog/oosplash.bin" "$@"
