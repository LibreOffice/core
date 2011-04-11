#!/bin/sh

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
