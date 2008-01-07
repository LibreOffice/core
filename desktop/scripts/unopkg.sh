#!/bin/sh
#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: unopkg.sh,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: obo $ $Date: 2008-01-07 09:56:38 $
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
    DYLD_LIBRARY_PATH=${sd_prog}${DYLD_LIBRARY_PATH+:${DYLD_LIBRARY_PATH}}
    export DYLD_LIBRARY_PATH
    ;;

  HP-UX)
    SHLIB_PATH=${sd_prog}:/usr/openwin/lib${SHLIB_PATH+:${SHLIB_PATH}}
    export SHLIB_PATH
    ;;

  IRIX*)
    LD_LIBRARYN32_PATH=${sd_prog}${LD_LIBRARYN32_PATH+:${LD_LIBRARYN32_PATH}}
    export LD_LIBRARYN32_PATH
    ;;

  *)
    LD_LIBRARY_PATH=${sd_prog}${LD_LIBRARY_PATH+:${LD_LIBRARY_PATH}}
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
unset java_ld_library_path
if [ -x "$sd_prog/javaldx" ] ; then
    java_ld_library_path=`"$sd_prog/javaldx" $BOOTSTRAPVARS`
elif [ -x "$sd_prog/../ure-link/javaldx" ] ; then
    java_ld_library_path=`"$sd_prog/../ure-link/javaldx" $BOOTSTRAPVARS`
fi
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

# misc. environment variables
OPENOFFICE_MOZILLA_FIVE_HOME="$sd_inst/program"
export OPENOFFICE_MOZILLA_FIVE_HOME

unset XENVIRONMENT

# uncomment line below to disable anti aliasing of fonts
# SAL_ANTIALIAS_DISABLE=true; export SAL_ANTIALIAS_DISABLE

# set path so that other apps can be started just by name
PATH="$sd_prog":$PATH
export PATH

# execute binary
exec "$sd_prog/$sd_binary" "$@"

