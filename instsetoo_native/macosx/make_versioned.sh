#!/bin/sh
#
#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: make_versioned.sh,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: vg $ $Date: 2007-02-26 14:56:27 $
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
