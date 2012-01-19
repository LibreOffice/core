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
INSTANCE_URN=

while [ $# -gt 0 ]
do
  case "$1" in
    -t)  TARGET_URN="$2"; shift;;
    -p)  PRODUCT_NAME="$2"; shift;;
    -e)  PRODUCT_VERSION="$2"; shift;;
    -i)  INSTANCE_URN="$2"; shift;;
    -P)  PARENT_PRODUCT_NAME="$2"; shift;;
    -S)  PRODUCT_SOURCE="$2"; shift;;
    --)  shift; break;;
    -*)
        echo >&2 \
        "usage: $0 -p <product name> -e <product version> -t <urn> -S <source> -P <parent product name> [-i <instance urn>]"
        exit 1;;
    *)  break;;
    esac
    shift
done

[ -x "$STCLIENT" ] || exit 1

# test if already registered
if [ ! -n $INSTANCE_URN ]; then
  TEST=`${STCLIENT} -f -t ${TARGET_URN}`; EXITCODE=$?

  # retry on unexpected error codes
  [ ${EXITCODE} -eq 0 -o  ${EXITCODE} -eq 225 ] || exit 1

  # early versions did not have a dedicated exitcode, so need to compare text output
  [ ${EXITCODE} -eq 225 -o "${TEST}" = "No records found" ] || echo "${TEST}"; exit 0
fi

uname=`uname -p`
zone="global"

if [ `uname -s` = "SunOS" ]; then
  if [ -x /usr/bin/zonename ]; then
    zone=`/usr/bin/zonename`
  fi
fi

output=`"${STCLIENT}" -a -p "${PRODUCT_NAME}" -e "${PRODUCT_VERSION}" -t ${TARGET_URN} -S "${PRODUCT_SOURCE}" -P "${PARENT_PRODUCT_NAME}" ${INSTANCE_URN:+"-i"} ${INSTANCE_URN} -m "Sun Microsystems, Inc." -A "${uname}" -z "${zone}"`; EXITCODE=$?

[ "${INSTANCE_URN}" = "" -a ${EXITCODE} -eq 226 ] && exit 0

exit ${EXITCODE}

