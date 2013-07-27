#!/bin/sh
#**************************************************************
#
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#
#**************************************************************

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
"$sd_prog/pagein" -L"$sd_prog" \
    ${sd_pagein_args}

# extend the ld_library_path for java: javaldx checks the sofficerc for us
if [ -x "$sd_prog/javaldx" ] ; then
    # this is a temporary hack until we can live with the default search paths
    case "`uname -s`" in
    FreeBSD)
    sd_prog1="$sd_prog"
    LD_LIBRARY_PATH=$sd_prog1:${LD_LIBRARY_PATH}}
    export LD_LIBRARY_PATH
    ;;
    esac
    my_path=`"$sd_prog/javaldx" $BOOTSTRAPVARS \
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
