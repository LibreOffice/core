#!/usr/bin/env bash

find browser/src/ -name "*.js" -exec grep ': function' \{\} \; | sed -e 's/:.*//' -e 's/^[\t ]*//' -e 's/[\t ]*$//' | grep -v " " | sort | uniq | \
    while read FUNC ; do
        NUM=`git grep "\<$FUNC\>" browser/src/ | wc -l`
        #echo "Trying $FUNC: $NUM"
        if [ "$NUM" = "1" ] ; then
            git --no-pager grep "\<$FUNC\>" browser/src/
        fi
    done
