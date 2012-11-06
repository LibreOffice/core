#!/bin/sh
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

linenum=LINENUMBERPLACEHOLDER

UNPACKDIR=/var/tmp/unpack_PRODUCTNAMEPLACEHOLDER
diskSpaceRequired=DISCSPACEPLACEHOLDER
checksum=CHECKSUMPLACEHOLDER

EXTRACTONLY="no"
if [ "$1" = "-x" ]
then
    EXTRACTONLY=yes
fi

# Determining current platform

platform=`uname -s`

case $platform in
SunOS)
  tail_prog="tail"
  ;;
Linux)
  tail_prog="tail -n"
  ;;
*)
  tail_prog="tail"
  ;;
esac

# Asking for the unpack directory

echo
echo "Select the directory in which to save the unpacked files. [$UNPACKDIR] "
read reply leftover
if [ "x$reply" != "x" ]
then
  UNPACKDIR="$reply"
fi

if [ -d $UNPACKDIR ]; then
    printf "Directory $UNPACKDIR already exists.\n"
    printf "Please select a new directory name.\n"
    exit 1
fi

# Unpacking

mkdir -m 700 $UNPACKDIR

diskSpace=`df -k $UNPACKDIR | $tail_prog -1 | awk '{if ( $4 ~ /%/) { print $3 } else { print $4 } }'`
if [ $diskSpace -lt $diskSpaceRequired ]; then
    printf "The selected drive does not have enough disk space available.\n"
    printf "PRODUCTNAMEPLACEHOLDER requires at least %s kByte.\n" $diskSpaceRequired
    exit 1
fi

trap 'rm -rf $UNPACKDIR; exit 1' HUP INT QUIT TERM

if [ -x /usr/bin/sum ] ; then
    echo "File is being checked for errors ..."

    sum=`$tail_prog +$linenum $0 | /usr/bin/sum`
    sum=`echo $sum | awk '{ print $1 }'`

    if [ $sum != $checksum ]; then
        echo "The download file appears to be corrupted. Please download PRODUCTNAMEPLACEHOLDER again."
        exit 1
    fi
fi

echo "Unpacking ..."

$tail_prog +$linenum $0 | (cd $UNPACKDIR; tar xf -)

echo "All files have been successfully unpacked."

if [ "$EXTRACTONLY" != "yes" ]
then
  if [ -f $UNPACKDIR/setup ]
  then
      chmod 775 $UNPACKDIR/setup
      $UNPACKDIR/setup
  fi
fi

exit 0
