#!/bin/sh
#
# Copyright (c) 2007 by Sun Microsystems, Inc.
# All rights reserved.
#

if [ `uname -s` = "SunOS" ]; then
  STCLIENT=/usr/bin/stclient
else
  STCLIENT=/opt/sun/servicetag/bin/stclient
fi

TARGET_URN=
PRODUCT_NAME=
PRODUCT_VERSION=
PRODUCT_SOURCE=
PARENT_PRODUCT_NAME=

while [ $# -gt 0 ]
do
  case "$1" in
    -t)  TARGET_URN="$2"; shift;;
    -p)  PRODUCT_NAME="$2"; shift;;
    -e)  PRODUCT_VERSION="$2"; shift;;
    -P)  PARENT_PRODUCT_NAME="$2"; shift;;
    -S)  PRODUCT_SOURCE="$2"; shift;;
    --)  shift; break;;
    -*)
        echo >&2 \
        "usage: $0 -p <product name> -e <product version> -t <urn> -S <source> -P <parent product name>"
        exit 1;;
    *)  break;;
    esac
    shift
done

[ -x "$STCLIENT" ] || exit 1

TEST=`${STCLIENT} -f -t ${TARGET_URN}` || exit 1
[ "${TEST}" = "No records found" ] || exit 0

uname=`uname -p`
zone="global"

if [ `uname -s` = "SunOS" ]; then
  if [ -x /usr/bin/zonename ]; then
    zone=`/usr/bin/zonename`
  fi
fi

output=`"${STCLIENT}" -a -p "${PRODUCT_NAME}" -e "${PRODUCT_VERSION}" -t "${TARGET_URN}" -S "${PRODUCT_SOURCE}" -P "${PARENT_PRODUCT_NAME}" -m "Sun Microsystems, Inc." -A "${uname}" -z "${zone}"` || exit 1

exit 0

