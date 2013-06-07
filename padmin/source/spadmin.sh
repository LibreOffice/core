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
    # this is a temporary hack until we can live with the default search paths
    case "`uname -s`" in
    FreeBSD)
    LD_LIBRARY_PATH=$sd_prog:${LD_LIBRARY_PATH+:${LD_LIBRARY_PATH}}
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

# Set PATH so that crash_report is found:
PATH=$sd_prog${PATH+:$PATH}
export PATH

# execute binary
exec "$sd_prog/spadmin.bin" "$@" \
    "-env:INIFILENAME=vnd.sun.star.pathname:$sd_prog/redirectrc"
