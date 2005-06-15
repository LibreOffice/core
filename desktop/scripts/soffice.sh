#!/bin/sh
#*************************************************************************
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc..
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

#
# STAR_PROFILE_LOCKING_DISABLED=1
# export STAR_PROFILE_LOCKING_DISABLED
#

# file locking now enabled by default
SAL_ENABLE_FILE_LOCKING=1
export SAL_ENABLE_FILE_LOCKING

# the following test is needed on Linux PPC with IBM j2sdk142
if [ `uname -s` = "Linux" -a  "`uname -m`" = "ppc" ] ; then
    JITC_PROCESSOR_TYPE=6
    export JITC_PROCESSOR_TYPE
fi

# set -x

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
moz_lib=
for moz_lib_path in $MOZILLA_LIBRARY_PATH /usr/lib /usr/lib/mozilla /usr/lib/mozilla-firefox /usr/lib/mozilla-thunderbird /opt/mozilla/lib /opt/MozillaFirefox/lib /opt/MozillaThunderbird/lib; do
    test -f $moz_lib_path/libnss3.so && moz_lib="$moz_lib_path" && break;
done
case $sd_platform in
  AIX)
    # this is a temporary hack until we can live with the default search paths
    if [ $LIBPATH ]; then
      SYSTEM_LIBPATH=$LIBPATH
      export SYSTEM_LIBPATH
      LIBPATH="$sd_prog:$moz_lib":$LIBPATH
    else
      LIBPATH="$sd_prog:$moz_lib"
    fi
    export LIBPATH
    ;;

  Darwin)
    # this is a temporary hack until we can live with the default search paths
    if [ $DYLD_LIBRARY_PATH ]; then
      SYSTEM_DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH
      export SYSTEM_DYLD_LIBRARY_PATH
      DYLD_LIBRARY_PATH="$sd_prog:$moz_lib":$DYLD_LIBRARY_PATH
    else
      DYLD_LIBRARY_PATH="$sd_prog:$moz_lib"
    fi
    export DYLD_LIBRARY_PATH
    ;;

  HP-UX)
    # this is a temporary hack until we can live with the default search paths
    if [ $SHLIB_PATH ]; then
      SYSTEM_SHLIB_PATH=$SHLIB_PATH
      export SYSTEM_SHLIB_PATH
      SHLIB_PATH="$sd_prog:$moz_lib":/usr/openwin/lib:$SHLIB_PATH
    else
      SHLIB_PATH="$sd_prog:$moz_lib":/usr/openwin/lib
    fi
    export SHLIB_PATH
    ;;

  IRIX*)
    # this is a temporary hack until we can live with the default search paths
    if [ $LD_LIBRARYN32_PATH ]; then
       SYSTEM_LD_LIBRARYN32_PATH=$LD_LIBRARYN32_PATH
       export SYSTEM_LD_LIBRARYN32_PATH
       LD_LIBRARYN32_PATH=:"$sd_prog:$moz_lib":$LD_LIBRARYN32_PATH
    else
       LD_LIBRARYN32_PATH=:"$sd_prog:$moz_lib"
    fi
    export LD_LIBRARYN32_PATH
    ;;

  *)
    # this is a temporary hack until we can live with the default search paths
    if [ $LD_LIBRARY_PATH ]; then
      SYSTEM_LD_LIBRARY_PATH=$LD_LIBRARY_PATH
      export SYSTEM_LD_LIBRARY_PATH
      LD_LIBRARY_PATH="$sd_prog:$moz_lib":$LD_LIBRARY_PATH
    else
      LD_LIBRARY_PATH="$sd_prog:$moz_lib"
    fi
    export LD_LIBRARY_PATH
    ;;
esac

# extend the ld_library_path for java: javaldx checks the sofficerc for us
if [ -x "$sd_prog/javaldx" ] ; then
    java_ld_library_path=`"$sd_prog/javaldx"`
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

# misc. environment variables
OPENOFFICE_MOZILLA_FIVE_HOME="$sd_inst/program"
export OPENOFFICE_MOZILLA_FIVE_HOME

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
"${sd_prog}"/pagein -L"${sd_prog}" ${sd_pagein_args}

# set path so that other apps can be started from soffice just by name
if [ $PATH ]; then
  PATH="$sd_prog":$PATH
else
  PATH="$sd_prog"
fi
export PATH

# execute soffice binary
"$sd_prog/$sd_binary" "$@"

while [ $? -eq 79 ]
do
    "$sd_prog/$sd_binary"
done

exit
