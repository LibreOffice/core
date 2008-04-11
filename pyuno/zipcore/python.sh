#!/bin/sh
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: python.sh,v $
#
# $Revision: 1.9 $
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

# resolve installation directory
sd_cwd="`pwd`"
if [ -h "$0" ] ; then
    sd_basename=`basename "$0"`
     sd_script=`ls -l "$0" | sed "s/.*${sd_basename} -> //g"`
     sd_sub=`echo $sd_script | cut -f1 -d/`
    if [ "$sd_sub" = ".." -a "$SO_MODE" = "" ]; then
        SO_MODE="remote"
    fi
    cd "`dirname "$0"`"
    cd "`dirname "$sd_script"`"
else
    cd "`dirname "$0"`"
fi

sd_prog="`pwd`"
sd_progsub=$sd_prog/$SO_MODE

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
  SunOS)
    LD_LIBRARY_PATH="$sd_progsub":"$sd_prog":/usr/openwin/lib:/usr/dt/lib:$LD_LIBRARY_PATH
    export LD_LIBRARY_PATH
    ;;

  AIX)
    LIBPATH="$sd_progsub":"$sd_prog":$LIBPATH
    export LIBPATH
    ;;

  HP-UX)
    SHLIB_PATH="$sd_progsub":"$sd_prog":/usr/openwin/lib:$SHLIB_PATH
    export SHLIB_PATH
    ;;

  IRIX*)
    LD_LIBRARYN32_PATH=:"$sd_progsub":"$sd_prog":$LD_LIBRARYN32_PATH
    export LD_LIBRARYN32_PATH
    ;;

  Darwin*)
    DYLD_LIBRARY_PATH="$sd_progsub":"$sd_prog":$DYLD_LIBRARY_PATH
    export DYLD_LIBRARY_PATH
    ;;

  *)
    LD_LIBRARY_PATH="$sd_progsub":"$sd_prog":$LD_LIBRARY_PATH
    export LD_LIBRARY_PATH
    ;;
esac

PYTHONPATH="$sd_prog":"$sd_prog/python-core-%%PYVERSION%%/lib":"$sd_prog/python-core-%%PYVERSION%%/lib/lib-dynload":"$sd_prog/python-core-%%PYVERSION%%/lib/lib-tk":"$sd_prog/python-core-%%PYVERSION%%/lib/site-packages":"$PYTHONPATH"
export PYTHONPATH

PYTHONHOME="$sd_prog"/python-core-%%PYVERSION%%
export PYTHONHOME

# set path so that other apps can be started from soffice just by name
PATH="$sd_prog":$PATH
export PATH
exec "$sd_prog/python.bin" "$@"

