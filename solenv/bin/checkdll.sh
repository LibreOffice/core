#! /bin/sh
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
# checkdll.sh - execute checkdll with all -L arguments to this script
#               prepended to LD_LIBRARY_PATH

set -- `getopt "L:" "$@"` ||  {
    echo "Usage: `basename $0` [-L library_path] <shared_library>" 1>&2
    exit 1
}

checkdll="$SOLARVERSION/$INPATH/bin/checkdll"

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
    AIX) case "${LIBPATH:+X}" in
        X) LIBPATH=$libpath:$LIBPATH;;
        *) LIBPATH=$libpath;;
        esac
        export LIBPATH;;
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
    for parameter in $*; do
        library=$parameter;
    done
    realname=`echo $library | sed "s/check_//"`
    if [ $library != $realname ]; then
        mv $library $realname
    fi
fi

exit 0

