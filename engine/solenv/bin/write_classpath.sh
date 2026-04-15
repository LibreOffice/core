#!/usr/bin/env bash

dest="$1"
shift
base='Class-Path: '

while [ "${1}" != "" ]; do
    p="$1"
    shift
    echo "$base $p" >> $dest
    base=' '
done

#echo "added classpath"
#cat $dest
#echo "==="
