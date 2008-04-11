#!/bin/sh
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: regcomp.sh,v $
#
# $Revision: 1.4 $
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

# set search path for shared libraries
sd_platform=`uname -s`
case $sd_platform in
  AIX)
    # this is a temporary hack until we can live with the default search paths
    if [ $LIBPATH ]; then
      SYSTEM_LIBPATH=$LIBPATH
      export SYSTEM_LIBPATH
    fi
    LIBPATH="$sd_prog":$LIBPATH
    export LIBPATH
    ;;

  Darwin)
    # this is a temporary hack until we can live with the default search paths
    if [ $DYLD_LIBRARY_PATH ]; then
      SYSTEM_DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH
      export SYSTEM_DYLD_LIBRARY_PATH
    fi
    DYLD_LIBRARY_PATH="$sd_prog":$DYLD_LIBRARY_PATH
    export DYLD_LIBRARY_PATH
    ;;

  HP-UX)
    # this is a temporary hack until we can live with the default search paths
    if [ $SHLIB_PATH ]; then
      SYSTEM_SHLIB_PATH=$SHLIB_PATH
      export SYSTEM_SHLIB_PATH
    fi
    SHLIB_PATH="$sd_prog":/usr/openwin/lib:$SHLIB_PATH
    export SHLIB_PATH
    ;;

  IRIX*)
    # this is a temporary hack until we can live with the default search paths
    if [ $LD_LIBRARYN32_PATH ]; then
       SYSTEM_LD_LIBRARYN32_PATH=$LD_LIBRARYN32_PATH
       export SYSTEM_LD_LIBRARYN32_PATH
    fi
    LD_LIBRARYN32_PATH=:"$sd_prog":$LD_LIBRARYN32_PATH
    export LD_LIBRARYN32_PATH
    ;;

  *)
    # this is a temporary hack until we can live with the default search paths
    if [ $LD_LIBRARY_PATH ]; then
      SYSTEM_LD_LIBRARY_PATH=$LD_LIBRARY_PATH
      export SYSTEM_LD_LIBRARY_PATH
    fi
    LD_LIBRARY_PATH="$sd_prog":$LD_LIBRARY_PATH
    export LD_LIBRARY_PATH
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
if [ -x "$sd_prog/javaldx" ] ; then
    java_ld_library_path=`"$sd_prog/javaldx" $BOOTSTRAPVARS`
    if [ "$java_ld_library_path" != "" ] ; then
        case $sd_platform in
            AIX)
                LIBPATH=${java_ld_library_path}:${LIBPATH}
                ;;
            Darwin)
                DYLD_LIBRARY_PATH=${java_ld_library_path}:${DYLD_LIBRARY_PATH}
                ;;
            HP-UX)
                SHLIB_PATH=${java_ld_library_path}:${SHLIB_PATH}
                ;;
            IRIX*)
                LD_LIBRARYN32_PATH=${java_ld_library_path}:${LD_LIBRARYN32_PATH}
                ;;
            *)
                LD_LIBRARY_PATH=${java_ld_library_path}:${LD_LIBRARY_PATH}
                ;;
        esac
    fi
fi



# execute binary
exec "$sd_prog/$sd_binary" "$@"

