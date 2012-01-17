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

# Pass -env arguments on to javaldx; needs to be fixed:
my_envargs=
for my_arg in "$@" ; do
  case ${my_arg} in
    -env:*) my_envargs="${my_envargs} ${my_arg}" ;;
  esac
done

# Extend the LD_LIBRARY_PATH for Java:
epath=`dirname "$0"`
if [ -x "${epath}/javaldx" ] ; then
    jpath=`"${epath}/javaldx" $my_envargs`
    if [ -n "${jpath}" ]; then
        LD_LIBRARY_PATH=${jpath}${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}
        export LD_LIBRARY_PATH
    fi
fi

exec "$0.bin" "$@"
