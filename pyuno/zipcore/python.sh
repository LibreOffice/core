#!/bin/sh
#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: python.sh,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: rt $ $Date: 2005-11-11 09:21:32 $
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

PYTHONPATH="$sd_prog":"$sd_prog/python-core-%%PYVERSION%%/lib":"$sd_prog/python-core-%%PYVERSION%%/lib/lib-dynload":"$sd_prog/python-core-%%PYVERSION%%/lib/lib-tk":"$PYTHONPATH"
export PYTHONPATH

PYTHONHOME="$sd_prog"/python-core-%%PYVERSION%%
export PYTHONHOME

# set path so that other apps can be started from soffice just by name
PATH="$sd_prog":$PATH
export PATH
exec "$sd_prog/python.bin" "$@"

