#!/bin/sh
# *************************************************************
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
# *************************************************************

EXTENSION=`expr "//$1" : '.*\/.*\.\(t.*\)'`
[ -z $EXTENSION ] && ( echo "Unable to determine file type"; exit 2 )

BASEDIR=`dirname "$1"`
FOLDER=`basename "$1" ".$EXTENSION"`
NUM=1

DESTPATH="$BASEDIR/$FOLDER"

while [ -d "$DESTPATH" ]; do
  NUM=`expr $NUM + 1`
  DESTPATH="$BASEDIR/$FOLDER-$NUM"
done

mkdir "$DESTPATH"
cd "$DESTPATH"

if [ "$EXTENSION" = "tar.gz" -o "$EXTENSION" = "tgz" ]; then
  if [ -x /usr/bin/gzcat ]; then
    /usr/bin/gzcat "$1" | tar -xf -
  else
    tar -xzf "$1"
  fi
elif [ "$EXTENSION" = "tar.bz2" -o "$EXTENSION" = "tbz2" ]; then
  /usr/bin/bzcat "$1" | tar -xf -
else
  echo "Unsupported type of archive"
  exit 2
fi

UPDATE=`eval ls */update` && SUBFOLDER=`dirname $UPDATE` && mv $SUBFOLDER/* . && rmdir $SUBFOLDER && echo "$DESTPATH/update"
