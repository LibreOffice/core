#!/bin/sh
#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: soffice.sh,v $
#
#   $Revision: 1.31 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 13:44:45 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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

# the following test is needed on Linux PPC with IBM j2sdk142
if [ "`uname -s`" = "Linux" -a "`uname -m`" = "ppc" ] ; then
    JITC_PROCESSOR_TYPE=6
    export JITC_PROCESSOR_TYPE
fi

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

sd_binary=`basename "$0"`.bin

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
        "-env:INIFILEPATH=$sd_prog/redirectrc"`
    if [ -n "$my_path" ] ; then
        LD_LIBRARY_PATH=$my_path${LD_LIBRARY_PATH+:$LD_LIBRARY_PATH}
        export LD_LIBRARY_PATH
    fi
fi

unset XENVIRONMENT

# uncomment line below to disable anti aliasing of fonts
# SAL_ANTIALIAS_DISABLE=true; export SAL_ANTIALIAS_DISABLE

# pagein
for sd_arg in ${1+"$@"} ; do
    case ${sd_arg} in
    -calc)
        sd_pagein_args="${sd_pagein_args:+${sd_pagein_args} }@pagein-calc"
        break;
        ;;
    -draw)
        sd_pagein_args="${sd_pagein_args:+${sd_pagein_args} }@pagein-draw"
        break;
        ;;
    -impress)
        sd_pagein_args="${sd_pagein_args:+${sd_pagein_args} }@pagein-impress"
        break;
        ;;
    -writer)
        sd_pagein_args="${sd_pagein_args:+${sd_pagein_args} }@pagein-writer"
        break;
        ;;
    *)
        ;;
    esac
done

# read database entries for Adabas D
if [ -f /etc/adabasrc ]; then
  . /etc/adabasrc
fi

sd_pagein_args="${sd_pagein_args:+${sd_pagein_args} }@pagein-common"
"$sd_prog/../basis-link/program/pagein" -L"$sd_prog/../basis-link/program" \
    ${sd_pagein_args}

# Set PATH so that crash_report is found:
PATH=$sd_prog${PATH+:$PATH}
export PATH

# execute soffice binary
"$sd_prog/$sd_binary" "$@" &
trap 'kill -9 $!' TERM
wait $!

while [ $? -eq 79 ]
do
    "$sd_prog/$sd_binary" ""$BOOTSTRAPVARS"" &
    wait $!
done

exit
