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

sd_prog="`pwd`"

cd ..
sd_binary=`basename "$0"`".bin"
sd_inst="`pwd`"

# change back directory
cd "$sd_cwd"

# check if all required patches are installed
if [ -x "$sd_prog/sopatchlevel.sh" ]; then
    "$sd_prog/sopatchlevel.sh"
    if [ $? -eq 1 ]; then
        exit 0
    fi
fi

# set search path for shared libraries
sd_platform=`uname -s`
case $sd_platform in
  AIX)
    LIBPATH=${sd_prog}${LIBPATH+:${LIBPATH}}
    export LIBPATH
    ;;

  Darwin)
    DYLD_LIBRARY_PATH=${sd_prog}${DYLD_LIBRARY_PATH:+:${DYLD_LIBRARY_PATH}}
    export DYLD_LIBRARY_PATH
    ;;

  HP-UX)
    SHLIB_PATH=${sd_prog}:/usr/openwin/lib${SHLIB_PATH:+:${SHLIB_PATH}}
    export SHLIB_PATH
    ;;

  *)
    LD_LIBRARY_PATH=${sd_prog}${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}
    export LD_LIBRARY_PATH
    ;;
esac

# misc. environment variables
unset XENVIRONMENT

# set path so that other apps can be started just by name
PATH="$sd_prog":$PATH
export PATH

OOO_INSTALL_PREFIX=${OOO_INSTALL_PREFIX-$sd_prog/../..}
if ! test -e $OOO_INSTALL_PREFIX/basis-link; then
    # Hack for vanilla OOo binaries' split install layout
    OOO_INSTALL_PREFIX=$OOO_INSTALL_PREFIX/../openoffice.org3
fi

export OOO_INSTALL_PREFIX

# execute binary
exec "$sd_prog/$sd_binary" "$@"
