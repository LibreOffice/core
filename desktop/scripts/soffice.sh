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

# The following is needed on Linux PPC with IBM j2sdk142:
#@# export JITC_PROCESSOR_TYPE=6

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

# pagein
sd_pagein_args=@pagein-common
for sd_arg in "$@"; do
    case ${sd_arg} in
    -calc)
        sd_pagein_args="${sd_pagein_args} @pagein-calc"
        break;
        ;;
    -draw)
        sd_pagein_args="${sd_pagein_args} @pagein-draw"
        break;
        ;;
    -impress)
        sd_pagein_args="${sd_pagein_args} @pagein-impress"
        break;
        ;;
    -writer)
        sd_pagein_args="${sd_pagein_args} @pagein-writer"
        break;
        ;;
    esac
done
"$sd_prog/../basis-link/program/pagein" -L"$sd_prog/../basis-link/program" \
    ${sd_pagein_args}

# extend the ld_library_path for java: javaldx checks the sofficerc for us
if [ -x "$sd_prog/../basis-link/ure-link/bin/javaldx" ] ; then
    my_path=`"$sd_prog/../basis-link/ure-link/bin/javaldx" $BOOTSTRAPVARS \
        "-env:INIFILENAME=vnd.sun.star.pathname:$sd_prog/redirectrc"`
    if [ -n "$my_path" ] ; then
        LD_LIBRARY_PATH=$my_path${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
        export LD_LIBRARY_PATH
    fi
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

# execute soffice binary
"$sd_prog/$sd_binary" "$@" &
trap 'kill -9 $!' TERM
wait $!
sd_ret=$?

while [ $sd_ret -eq 79 -o $sd_ret -eq 81 ]
do
    if [ $sd_ret -eq 79 ]; then
        "$sd_prog/$sd_binary" ""$BOOTSTRAPVARS"" &
    elif [ $sd_ret -eq 81 ]; then
        "$sd_prog/$sd_binary" "$@" &
    fi

    wait $!
    sd_ret=$?
done

exit $sd_ret
