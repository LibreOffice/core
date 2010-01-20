#!/bin/sh
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.8 $
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

if [ -z "$TARFILE_LOCATION" ]; then
    echo "ERROR: no destination defined! please set TARFILE_LOCATION!"
    exit
fi

if [ ! -d "$TARFILE_LOCATION" ]; then
    mkdir $TARFILE_LOCATION
fi
if [ ! -d "$TARFILE_LOCATION" ]; then
    echo "ERROR: can't create"
    exit
fi

if [ -z "$1" ]; then
    echo "ERROR: parameter missing!"
    echo "usage: $0 <fetch list>"
    echo "first line must define the base url."
    exit
fi

# check for wget and md5sum
wget=
md5sum=

for i in wget /usr/local/bin/wget /usr/sfw/bin/wget /opt/sfw/bin/wget; do
    eval "$i --version" > /dev/null 2>&1
    ret=$?
    if [ $ret -eq 0 ]; then
        wget=$i
        echo found wget $wget
        break 2
    fi
done

if [ -z "$wget" ]; then
    echo "ERROR: no wget found!"
    exit
fi

for i in md5sum /usr/local/bin/md5sum gmd5sum /usr/sfw/bin/md5sum /opt/sfw/bin/gmd5sum; do
    eval "$i --version" > /dev/null 2>&1
    ret=$?
    if [ $ret -eq 0 ]; then
        md5sum=$i
        echo found md5sum: $md5sum
        break 2
    fi
done

if [ -z "$md5sum" ]; then
    echo "ERROR: no md5sum: found!"
    exit
fi

start_dir=`pwd`
for i in `cat $1` ; do
#   echo $i
    if [ "$i" != `echo $i | sed "s/^http:\///"` ]; then
        tarurl=$i
    # TODO: check for comment
    else
        if [ "$tarurl" != "" ]; then
            cd $TARFILE_LOCATION
            $wget -nv -N $tarurl/$i
            wret=$?
            if [ $wret -ne 0 ]; then
                failed="$failed $i"
                wret=0
            fi
            if [ -f $i ]; then
                sum=`$md5sum $i | sed "s/ [ *].*//"`
                sum2=`echo $i | sed "s/-.*//"`
                if [ "$sum" != "$sum2" ]; then
                    echo checksum failure for $i
                    failed="$failed $i"
                fi
            fi
            cd $start_dir
        fi
    fi
done

if [ ! -z "$failed" ]; then
    echo $failed | sed "s/ /\n/g" | sed "s/^/ERROR: failed to download: /"
fi

