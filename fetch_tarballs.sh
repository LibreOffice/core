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

file_list_name=$1

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
fetch_args=

#Look for FreeBSD's fetch(1) first
if [ -x /usr/bin/fetch ]; then
    fetch_bin=/usr/bin/fetch
    fetch_args="-AFpr"
    echo found FreeBSD fetch: $fetch_bin
    break 2
else
  for wg in wget /usr/bin/wget /usr/local/bin/wget /usr/sfw/bin/wget /opt/sfw/bin/wget /opt/local/bin/wget; do
    eval "$wg --version" > /dev/null 2>&1
    ret=$?
    if [ $ret -eq 0 ]; then
        fetch_bin=$wg
        fetch_args="-nv -N"
        echo found wget at `which $fetch_bin`
        break 2
    fi
  done
  if [ -z "$fetch_bin" ]; then
    for c in curl /usr/bin/curl /usr/local/bin/curl /usr/sfw/bin/curl /opt/sfw/bin/curl /opt/local/bin/curl; do
    # mac curl returns "2" on --version
    #    eval "$i --version" > /dev/null 2>&1
    #    ret=$?
    #    if [ $ret -eq 0 ]; then
        if [ -x $c ]; then
            fetch_bin=$c
            fetch_args="$file_date_check -O"
            echo found curl at `which $fetch_bin`
            break 2
        fi
    done
  fi
  if [ -z "$fetch_bin" ]; then
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
        echo found md5sum at `which $md5sum`
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


basename ()
{
    echo $1 | sed "s/^\(.*\/\)//"
}


#
# Download a file from a URL and add its md5 checksum to its name.
#
download ()
{
    local URL=$1

    if [ -n "$URL" ]; then
        local basename=$(basename $URL)
        local candidate=$(find "$TARFILE_LOCATION" -type f -name "*-$basename")
        if [ -n "$candidate" ]; then
            echo "$basename is already present ($candidate)"
        else
            echo fetching $basename
            $fetch_bin $fetch_args $URL 2>&1 | tee -a $logfile

            if [ $? -ne 0 ]; then
                echo "download failed"
                mv $basename ${basename}_broken
                failed="$failed $i"
            elif [ -f "$basename" -a -n "$md5sum" ]; then
                local sum=`$md5sum $md5special $basename | sed "s/ .*//"`
                mv $basename "$TARFILE_LOCATION/$sum-$basename"
                echo "added md5 sum $sum"
            fi
        fi
    fi
}

#
# Download a file from a URL and check its md5 sum to the one that is part of its name.
#
download_and_check ()
{
    local URL=$1

    if [ -n "$URL" ]; then
        local basename=$(basename $URL)
        if [ -f "$TARFILE_LOCATION/$basename" ]; then
            echo "$basename is already present"
        else
            echo "fetching $basename"
            $fetch_bin $fetch_args $URL 2>&1 | tee -a $logfile

            if [ $? -ne 0 ]; then
                echo "download failed"
                mv $basename ${basename}_broken
                failed="$failed $i"
            elif [ -f "$basename" -a -n "$md5sum" ]; then
                local sum=`$md5sum $md5special $basename | sed "s/ .*//"`
                local sum_in_name=`echo $basename | sed "s/-.*//"`
                if [ "$sum" != "$sum_in_name" ]; then
                    echo checksum failure for $basename 2>&1 | tee -a $logfile
                    failed="$failed $basename"
                    mv $basename ${basename}_broken
                fi
                mv $basename "$TARFILE_LOCATION/$basename"
            fi
        fi
    fi
}

echo "downloading tar balls to $TARFILE_LOCATION"

while read line ; do
    # Remove leading and trailing space and comments
    line=`echo $line | sed 's/^[[:space:]]*//;s/[[:space:]]*$//;s/[[:space:]]*#.*$//'`
    case $line in
        # Ignore empty lines.
        '')
            ;;

        # When a URL ends in a / then it is taken as a partial URL
        # to which the following lines will be appended.
        ftp:\/\/*\/ | http:\/\/*\/)
            UrlHead=$line
            echo $UrlHead
            ;;

        # A full URL represents a single file which is downloaded.
        ftp:\/\/* | http:\/\/*)
            download $line
            ;;

        # Any other line is interpreted as the second part of a partial URL.
        # It is appended to UrlHead and then downloaded.
        *)
            download_and_check $UrlHead$line
            ;;
    esac
done < "$file_list_name"


# Special handling of dmake
if [ -n "$DMAKE_URL" -a ! -x "$SOLARENV/$OUTPATH/bin/dmake$EXEEXT" ]; then
    download $DMAKE_URL
fi

# Special handling of epm-3.7
# Basically just a download of the epm archive.
# When its name contains "-source" than that part is removed.
epm_archive_tail=`echo $(basename $EPM_URL) | sed 's/-source//'`
epm_archive_name=$(find "$TARFILE_LOCATION" -type f -name "*-$epm_archive_tail")
if [ -n "$EPM_URL" -a ! -x "$SOLARENV/$OUTPATH/bin/epm$EXEEXT" -a -z "$epm_archive_name" ]; then
    download $EPM_URL
    archive_name=$(find "$TARFILE_LOCATION" -type f -name "*-epm-3.7-source*")
    if [ -n "$archive_name" ]; then
        epm_archive_name=`echo $archive_name | sed 's/-source//'`
        mv "$archive_name" "$epm_archive_name"
    fi
fi

if [ ! -z "$failed" ]; then
    echo
    echo ERROR: failed on:
    for i in $failed ; do
        echo $i
    done
    exit 1
fi

