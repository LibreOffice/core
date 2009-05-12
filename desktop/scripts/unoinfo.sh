#!/bin/sh
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: unoinfo.sh,v $
#
# $Revision: 1.2 $
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

set -e

# resolve installation directory
if [ -h "$0" ] ; then
    sd_basename=`basename "$0"`
    sd_script=`ls -l "$0" | sed "s/.*${sd_basename} -> //g"`
    cd "`dirname "$0"`"
    cd "`dirname "$sd_script"`"
else
    cd "`dirname "$0"`"
fi
sd_prog=`pwd`

case $1 in
c++)
    printf '%s' "$sd_prog/../basis-link/ure-link/lib"
    ;;
java)
    printf '0%s\0%s\0%s\0%s\0%s' \
        "$sd_prog/../basis-link/ure-link/share/java/ridl.jar" \
        "$sd_prog/../basis-link/ure-link/share/java/jurt.jar" \
        "$sd_prog/../basis-link/ure-link/share/java/juh.jar" \
        "$sd_prog/../basis-link/program/classes/unoil.jar" "$sd_prog"
    ;;
*)
    exit 1
    ;;
esac
