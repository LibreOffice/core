#!/bin/sh -x
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

# Downloader method selection
fetch_bin=
#fetch_args=

#Look for FreeBSD's fetch(1) first
if [ -x /usr/bin/fetch ]; then
    fetch_bin=/usr/bin/fetch
    fetch_args="-AFpr"
    echo found FreeBSD fetch: $fetch_bin
    break 2
else
  for wg in wget /usr/bin/wget /usr/local/bin/wget /usr/sfw/bin/wget /opt/sfw/bin/wget /opt/local/bin/wget; do
    eval "$i --version" > /dev/null 2>&1
    ret=$?
    if [ $ret -eq 0 ]; then
        fetch_bin=$wg
    fetch_args="-nv -N"
        echo found wget: $fetch_bin
        break 2
    fi
  done
  if [ -z "$fetch_bin" ]; then
    for c in curl /usr/bin/curl /usr/local/bin/curl /usr/sfw/bin/curl /opt/sfw/bin/curl /opt/local/bin/curl; do
    # mac curl returns "2" on --version
    #    eval "$i --version" > /dev/null 2>&1
    #    ret=$?
    #    if [ $ret -eq 0 ]; then
        if [ -x $i ]; then
            fetch_bin=$c
        fetch_args="$file_date_check -O"
            echo found curl: $fetch_bin
            break 2
        fi
    done
  fi
  if [ -z "$fetch_bin"]; then
    echo "ERROR: neither wget nor curl found!"
    exit
  fi
fi

#Checksummer selection
md5sum=

for i in md5 md5sum /usr/local/bin/md5sum gmd5sum /usr/sfw/bin/md5sum /opt/sfw/bin/gmd5sum /opt/local/bin/md5sum; do
    if [ "$i" = "md5" ]; then
        eval "$i -x" > /dev/null 2>&1
    else
        eval "$i --version" > /dev/null 2>&1
    fi
    ret=$?
    if [ $ret -eq 0 ]; then
        md5sum=$i
        echo found md5sum: $md5sum
        break 2
    fi
done

if [ "$md5sum" = "md5" ]; then
    md5special=-r
fi

if [ -z "$md5sum" ]; then
    echo "Warning: no md5sum: found!"
fi

start_dir=`pwd`
logfile=$TARFILE_LOCATION/fetch.log
date >> $logfile

# Create and go to a temporary directory under the tar file destination.
mkdir -p $TARFILE_LOCATION/tmp
cd $TARFILE_LOCATION/tmp

if [ -n "$DMAKE_URL" -a ! -x "$SOLARENV/$OUTPATH/bin/dmake$EXEEXT" ]; then
    # Determine the name of the downloaded file.
    dmake_package_name=`echo $DMAKE_URL | sed "s/^\(.*\/\)//"`

    if [ ! -f "../$dmake_package_name" ]; then
        # Fetch the dmake source
        echo fetching $DMAKE_URL to $TARFILE_LOCATION/tmp
        $fetch_bin $fetch_args $DMAKE_URL 2>&1 | tee -a $logfile
        wret=$?

        # When the download failed then remove the remains, otherwise
        # move the downloaded file up to TARFILE_LOCATION
        if [ $wret -ne 0 ]; then
            echo "download failed. removing $dmake_package_name"
            rm "$dmake_package_name"
            failed="$failed $i"
            wret=0
        else
            mv "$dmake_package_name" ..
            echo "successfully downloaded $dmake_package_name"
        fi
    else
        echo "found $dmake_package_name, no need to download it again"
    fi
fi


#Special handling of epm
if [ -n "$EPM_URL" -a ! -x "$SOLARENV/$OUTPATH/bin/epm$EXEEXT" ]; then
    # Determine the name of the downloaded file.
    epm_package_name=`echo $EPM_URL | sed "s/^\(.*\/\)//"`
    epm_package=`echo $epm_package_name | sed "s/-source//"`
    epm_wildcard_package_name="*-$epm_package"

    epmtest=$(find .. -type f -name "$epm_wildcard_package_name")

    # check with wildcard for the renamed package, md5
    if [ -z "$epmtest" ]; then
        # Fetch the epm source
          echo fetching $EPM_URL to $TARFILE_LOCATION/tmp
          $fetch_bin $fetch_args $EPM_URL 2>&1 | tee -a $logfile

        wret=$?

        # When the download failed then remove the remains, otherwise
        # move the downloaded file up to TARFILE_LOCATION and rename it
    # according our naing convention for external tar balls.
        if [ $wret -ne 0 ]; then
            echo "download failed. removing $epm_package_name"
            rm "$epm_package_name"
            failed="$failed $i"
            wret=0
        else
            #mv "$epm_package_name" ..
        epm_md5_sum=`$md5sum $md5special $epm_package_name | sed "s/ .*//"`
        epm_md5_package_name="$epm_md5_sum-$epm_package"

        rm -f ../$epm_md5_package_name && \
        cp -pRP $epm_package_name ../$epm_md5_package_name && \
        rm -rf $epm_package_name
            echo "successfully downloaded $epm_package_name and renamed to $epm_md5_package_name"
        fi
    else
        echo "found $epmtest, no need to download it again"
    fi
fi
# end special


cd $TARFILE_LOCATION/tmp
filelist=`cat $1`
echo $$ > fetch-running
for i in $filelist ; do
#    echo $i
    if [ "$i" != `echo $i | sed "s/^http:\///"` ]; then
        tarurl=$i
    # TODO: check for comment
    else
        if [ "$tarurl" != "" ]; then
            if [ ! -f "../$i" ]; then
                echo $i
        echo fetching $i
        $fetch_bin $fetch_args $tarurl/$i 2>&1 | tee -a $logfile
                wret=$?
                if [ -f $i -a -n "$md5sum" ]; then
                    sum=`$md5sum $md5special $i | sed "s/ .*//"`
                    sum2=`echo $i | sed "s/-.*//"`
                    if [ "$sum" != "$sum2" ]; then
                        echo checksum failure for $i 2>&1 | tee -a $logfile
                        failed="$failed $i"
                        mv $i ${i}_broken
                    else
                        mv $i ..
                    fi
                else
                    mv $i ..
                fi
            fi
        fi
    fi
done
rm $TARFILE_LOCATION/tmp/*-*
cd $start_dir

if [ ! -z "$failed" ]; then
    echo
    echo ERROR: failed on:
    for i in $failed ; do
        echo $i
    done
    exit 1
fi

