#! /bin/sh
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
# checkdll.sh - execute checkdll with all -L arguments to this script
#               prepended to LD_LIBRARY_PATH

set -- `getopt "L:" "$@"` ||  {
    echo "Usage: `basename $0` [-L library_path] <shared_library>" 1>&2
    exit 1
}

checkdll="$SOLARVERSION/$INPATH/bin$UPDMINOREXT/checkdll"

if [ -x $checkdll ]; then
    while :
    do
    case $1 in
        -L) shift; option=$1;;
        --) break;;
    esac
    case "${libpath:+X}" in
        X) libpath=$libpath:$option;;
        *) libpath=$option;;
    esac
    shift
    done
    shift  # remove the trailing ---

    case `uname -s` in
    Darwin) case "${DYLD_LIBRARY_PATH:+X}" in
        X) DYLD_LIBRARY_PATH=$libpath:$DYLD_LIBRARY_PATH;;
        *) DYLD_LIBRARY_PATH=$libpath;;
        esac
        export DYLD_LIBRARY_PATH;;
    *)  case "${LD_LIBRARY_PATH:+X}" in
        X) LD_LIBRARY_PATH=$libpath:$LD_LIBRARY_PATH;;
        *) LD_LIBRARY_PATH=$libpath;;
        esac
        export LD_LIBRARY_PATH;;
    esac

    $checkdll "$@"
    if [ $? -ne 0 ]; then exit 1 ; fi

    for parameter in $*; do
        library=$parameter;
    done
    realname=`echo $library | sed "s/check_//"`
    if [ $library != $realname ]; then
        LD_LIBRARY_PATH=
        export LD_LIBRARY_PATH
        mv $library $realname
    fi
else
    echo "WARNING: checkdll not found!" 1>&2
fi

exit 0

