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

# enable file locking
SAL_ENABLE_FILE_LOCKING=1
export SAL_ENABLE_FILE_LOCKING

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

# this is a temporary hack until we can live with the default search paths
case "`uname -s`" in
NetBSD|OpenBSD|FreeBSD|DragonFly)
    sd_prog1="$sd_prog"
    sd_prog2="$sd_prog/../ure-link/lib"
    LD_LIBRARY_PATH=$sd_prog1:$sd_prog2${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}
    JAVA_HOME=$(javaPathHelper -h libreoffice-java 2> /dev/null)
    export LD_LIBRARY_PATH
    if [ -n "${JAVA_HOME}" ]; then
        export JAVA_HOME
    fi
    ;;
AIX)
    sd_prog1="$sd_prog"
    sd_prog2="$sd_prog/../ure-link/lib"
    LIBPATH=$sd_prog1:$sd_prog2${LIBPATH:+:${LIBPATH}}
    export LIBPATH
    ;;
esac

#collect all bootstrap variables specified on the command line
#so that they can be passed as arguments to javaldx later on
for arg in $@
do
  case "$arg" in
       -env:*) BOOTSTRAPVARS=$BOOTSTRAPVARS" ""$arg";;
       -v) VERBOSE=true;;
       --verbose) VERBOSE=true;;
  esac
done

# extend the ld_library_path for java: javaldx checks the sofficerc for us
if [ -x "$sd_prog/../ure-link/bin/javaldx" ] ; then
    my_path=`"$sd_prog/../ure-link/bin/javaldx" $BOOTSTRAPVARS \
        "-env:INIFILENAME=vnd.sun.star.pathname:$sd_prog/redirectrc"`
    if [ -n "$my_path" ] ; then
        sd_platform=`uname -s`
        case $sd_platform in
          AIX)
            LIBPATH=$my_path${LIBPATH:+:$LIBPATH}
            export LIBPATH
            ;;
          *)
            LD_LIBRARY_PATH=$my_path${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
            export LD_LIBRARY_PATH
            ;;
        esac
    fi
fi

unset XENVIRONMENT

# uncomment line below to disable anti aliasing of fonts
# SAL_ANTIALIAS_DISABLE=true; export SAL_ANTIALIAS_DISABLE

# uncomment line below if you encounter problems starting soffice on your system
# SAL_NO_XINITTHREADS=true; export SAL_NO_XINITTHREADS

# execute binary
exec "$sd_prog/unopkg.bin" "$@" \
    "-env:INIFILENAME=vnd.sun.star.pathname:$sd_prog/redirectrc"

