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

if test -z "$HOME"; then
    HOME=$(getent passwd $(whoami) | cut -d":" -f6)
fi

# helper functions
home_on_nfs()
{
    case $(stat -f -c %T $HOME) in cifs|nfs|smb)
        return 0
    esac
    return 1
}
file_on_nfs(){
    for i; do
       case "$i" in -*) continue; esac
       [ -f "$i" ] || continue
       case $(stat -f -c %T "$i") in cifs|nfs|smb)
            return 0
       esac
    done
    return 1
}

# FIXME: this is conservative; allow more known working configurations
working_opengl_support()
{
    if [ -n "$(lsmod | awk '/^(fglrx|nvidia)/ {print $1}')" ]; then
       return 1
    fi
    return 0
}

# read config file

FILE_LOCKING=auto
OPENGL_SUPPORT=no
if [ -f /etc/libreoffice/soffice.sh ]; then
    . /etc/libreoffice/soffice.sh
fi

# sanity checks

case "$FILE_LOCKING" in
    auto|yes|no) ;;
    *)
        echo >&2 "unknown value '$FILE_LOCKING' for FILE_LOCKING parameter."
       FILE_LOCKING=auto
       echo >&2 "FILE_LOCKING reset to '$FILE_LOCKING'"
esac
 
case "$OPENGL_SUPPORT" in
    auto|yes|no) ;;
    *)
        echo >&2 "unknown value '$OPENGL_SUPPORT' for OPENGL_SUPPORT parameter."
       OPENGL_SUPPORT=auto
       echo >&2 "OPENGL_SUPPORT reset to '$OPENGL_SUPPORT'"
esac

# adjust environment

if [ -z "$SAL_ENABLE_FILE_LOCKING" ]; then
    case "$FILE_LOCKING" in
       auto)
        home_on_nfs "$@"
        if [ $? = 0 ]; then
           STAR_PROFILE_LOCKING_DISABLED=1
           export STAR_PROFILE_LOCKING_DISABLED
       fi
       file_on_nfs "$@"
       if [ $? = 0 ]; then
           SAL_ENABLE_FILE_LOCKING=0
           export SAL_ENABLE_FILE_LOCKING
           # for safety
           STAR_ENABLE_FILE_LOCKING=0
           export STAR_ENABLE_FILE_LOCKING
       else
            # file locking now enabled by default
           SAL_ENABLE_FILE_LOCKING=1
           export SAL_ENABLE_FILE_LOCKING
       fi
        ;;
       yes)
       SAL_ENABLE_FILE_LOCKING=1
       export SAL_ENABLE_FILE_LOCKING
        ;;
       no)
       SAL_ENABLE_FILE_LOCKING=0
       export SAL_ENABLE_FILE_LOCKING
       # for safety
       STAR_ENABLE_FILE_LOCKING=0
       export STAR_ENABLE_FILE_LOCKING
       STAR_PROFILE_LOCKING_DISABLED=1
       export STAR_PROFILE_LOCKING_DISABLED
    esac
fi

if [ -z "$SAL_NOOPENGL" ]; then
    case "$OPENGL_SUPPORT" in
       auto)
        working_opengl_support
        if [ $? -eq 0 ]; then
           SAL_NOOPENGL=true
           export SAL_NOOPENGL
       fi
        ;;
       yes)
       :
       unset SAL_NOOPENGL
       #export SAL_NOOPENGL
        ;;
       no)
       SAL_NOOPENGL=true
       export SAL_NOOPENGL
    esac
fi

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
    G_SLICE=always-malloc
    export G_SLICE
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
