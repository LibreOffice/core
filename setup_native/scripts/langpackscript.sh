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

MYUID=`id | sed "s/(.*//g" | sed "s/.*=//"`

if [ $MYUID -ne 0 ]
then
    echo You need to have super-user rights to install this language package
    exit 1
fi

linenum=LINENUMBERPLACEHOLDER

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

more << "EOF"
LICENSEFILEPLACEHOLDER
EOF

agreed=
while [ x$agreed = x ]; do
    echo
    echo "Do you agree to the above license terms? [yes or no] "
    read reply leftover
    case $reply in
    y* | Y*)
        agreed=1;;
    n* | N*)
    echo "If you don't agree to the license you can't install this software";
    exit 1;;
    esac
done

case $platform in
SunOS)
  SEARCHPACKAGENAME="BASISPACKAGEPREFIXPLACEHOLDERPRODUCTVERSIONPLACEHOLDER-core01"
  echo
  echo "Searching for the FULLPRODUCTNAMELONGPLACEHOLDER installation ..."
  PACKAGENAME=`pkginfo -x | grep $SEARCHPACKAGENAME | sed "s/ .*//"`
  if [ "x$PACKAGENAME" != "x" ]
  then
    PRODUCTINSTALLLOCATION="`pkginfo -r $PACKAGENAME`"
  else
    echo "FULLPRODUCTNAMELONGPLACEHOLDER not installed (no package $SEARCHPACKAGENAME installed)"
    exit 1
  fi
  ;;
Linux)
  SEARCHPACKAGENAME="BASISPACKAGEPREFIXPLACEHOLDERPRODUCTVERSIONPLACEHOLDER-core01"
  FIXPATH="/openoffice.org"
  echo
  echo "Searching for the FULLPRODUCTNAMELONGPLACEHOLDER installation ..."
  RPMNAME=`rpm -qa | grep $SEARCHPACKAGENAME`
  if [ "x$RPMNAME" != "x" ]
  then
    PRODUCTINSTALLLOCATION="`rpm -ql $RPMNAME | head -n 1`"
  else
    echo "FULLPRODUCTNAMELONGPLACEHOLDER not installed (no package $SEARCHPACKAGENAME installed)"
    exit 1
  fi
  PRODUCTINSTALLLOCATION=`echo $PRODUCTINSTALLLOCATION | sed "s#${FIXPATH}##"`
  ;;
*)
  echo "Unsupported platform"
  exit 1
  ;;
esac

# Asking for the installation directory

# echo
# echo "Where do you want to install the language pack ? [$PRODUCTINSTALLLOCATION] "
# read reply leftover
# if [ "x$reply" != "x" ]
# then
#   PRODUCTINSTALLLOCATION="$reply"
# fi

# Unpacking

outdir=/var/tmp/install_$$
mkdir $outdir

#diskSpace=`df -k $outdir | $tail_prog -1 | awk '{if ( $4 ~ /%/) { print $3 } else { print $4 } }'`
#if [ $diskSpace -lt $diskSpaceRequired ]; then
#    printf "You will need at least %s kBytes of free disk space\n" $diskSpaceRequired
#    printf "Please free up the required disk space and try again\n"
#    exit 3
#fi

trap 'rm -rf $outdir; exit 1' HUP INT QUIT TERM
echo "Unpacking and installing..."

#if [ -x /usr/bin/sum ] ; then
#    echo "Checksumming..."
#
#    sum=`/usr/bin/sum $outdir/$outname`
#    index=1
#    for s in $sum
#    do
#   case $index in
#   1)  sum1=$s;
#     index=2;
#       ;;
#   2)  sum2=$s;
#       index=3;
#       ;;
#   esac
#    done
#    if expr $sum1 != <sum1replace> || expr $sum2 != <sum2replace> ; then
#   echo "The download file appears to be corrupted.  Please refer"
#   echo "to the Troubleshooting section of the Installation"
#   exit 1
#   fi
#else
#    echo "Can't find /usr/bin/sum to do checksum.  Continuing anyway."
#fi

case $platform in
SunOS)
  $tail_prog +$linenum $0 | gunzip | (cd $outdir; tar xvf -)
  adminfile=$outdir/admin.$$
  echo "basedir=$PRODUCTINSTALLLOCATION" > $adminfile
INSTALLLINES
  ;;
Linux)
  $tail_prog +$linenum $0 | gunzip | (cd $outdir; tar xvf -)
INSTALLLINES
  ;;
*)
  echo "Unsupported platform"
  exit 1
  ;;
esac

rm -rf $outdir

echo "Done..."

exit 0
