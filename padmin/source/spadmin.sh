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
sd_cwd="`pwd`"
if [ -h "$0" ] ; then
    sd_basename=`basename "$0"`
     sd_script=`ls -l "$0" | sed "s/.*${sd_basename} -> //g"`
    cd "`dirname "$0"`"
    cd "`dirname "$sd_script"`"
else
    cd "`dirname "$0"`"
fi
sd_prog=`pwd`
cd "$sd_cwd"

case "`uname -s`" in
    FreeBSD)
        sd_prog1="$sd_prog/../basis-link/program"
        sd_prog2="$sd_prog/../basis-link/ure-link/lib"
        LD_LIBRARY_PATH=$sd_prog1:$sd_prog2${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
        export LD_LIBRARY_PATH
        ;;
    AIX)
        sd_prog1="$sd_prog/../basis-link/program"
        sd_prog2="$sd_prog/../basis-link/ure-link/lib"
        LIBPATH=$sd_prog1:$sd_prog2${LIBPATH:+:$LIBPATH}
        export LIBPATH
        ;;
esac

#collect all bootstrap variables specified on the command line
#so that they can be passed as arguments to javaldx later on
for arg in $@
do
  case "$arg" in
       -env:*) BOOTSTRAPVARS=$BOOTSTRAPVARS" ""$arg";;
  esac
done

# extend the ld_library_path for java: javaldx checks the sofficerc for us
if [ -x "$sd_prog/../basis-link/ure-link/bin/javaldx" ] ; then
    my_path=`"$sd_prog/../basis-link/ure-link/bin/javaldx" $BOOTSTRAPVARS \
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

# Set PATH so that crash_report is found:
PATH=$sd_prog${PATH+:$PATH}
export PATH

# execute binary
exec "$sd_prog/../basis-link/program/spadmin.bin" "$@" \
    "-env:INIFILENAME=vnd.sun.star.pathname:$sd_prog/redirectrc"
