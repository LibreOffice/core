#!/bin/sh

if [ -x /usr/bin/mktemp ]
then
  TMPFILE=`mktemp -t open-url.XXXXXX`
else
  DTTMPDIR=`xrdb -query | grep DtTmpDir`
  TMPFILE=${DTTMPDIR:-$HOME/.dt/tmp}/open-url.$$
fi

if [ -z "$TMPFILE" ]; then exit 1; fi
( echo "$1" > "$TMPFILE"; dtaction Open "$TMPFILE"; rm -f "$TMPFILE" ) &
exit 0
