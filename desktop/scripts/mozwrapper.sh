#!/bin/sh

# if mozilla is not found, specify full path here
MOZILLA=mozilla

if ${MOZILLA} -remote "openURL($1,new-window)" 2>&1 | egrep -si "not running on display"; then
    ${MOZILLA} $1
fi
