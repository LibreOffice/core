#!/bin/sh
#
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: make_versioned.sh,v $
#
# $Revision: 1.3 $
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
#
# Make versioned
# Uses openoffice.lst to create versioned files for Mac OS X
#

# version commands, similar to other OOo code
MAJOR=`sed -n '/^OpenOffice$/,/^}$/ s/.*USERDIRPRODUCTVERSION //p' ../util/openoffice.lst`
MAJOR_MINOR=`sed -n '/^OpenOffice$/,/PRODUCTVERSION/ s/.*PRODUCTVERSION //p' ../util/openoffice.lst`
MAJOR_MINOR_STEP=`sed -n '/^OpenOffice$/,/^}$/ s/.*ABOUTBOXPRODUCTVERSION //p' ../util/openoffice.lst`


if [ -z "$1" -o -z "$2" ]; then
  echo "ERROR: missing argument(s):";
  echo ""
  echo "USAGE: $0 <source file> <target file>"
  echo "   <source file>    File to be versioned"
  echo "   <target file>    Path and filename where to put the versioned file"
  exit
fi

if [ ! -e "$1" ]; then
  echo "ERROR: source file not found"
  exit
fi

TARGET_PATH=`dirname "$2"`
mkdir -p "$TARGET_PATH"

echo "make_versioned.sh: Using versions $MAJOR - $MAJOR_MINOR - $MAJOR_MINOR_STEP for $1, storing to $TARGET_PATH"

sed -e "s/\%USERDIRPRODUCTVERSION/${MAJOR}/g" "$1" |           \
sed -e "s/\%PRODUCTVERSION/${MAJOR_MINOR}/g" |                 \
sed -e "s/\%ABOUTBOXPRODUCTVERSION/${MAJOR_MINOR_STEP}/g" >"$2"
