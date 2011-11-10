#!/bin/sh
#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
#


#
# cd to build directory
#
cd ooo/main

date && echo "downloading additional and missing files"
wget -O external/unowinreg/unowinreg.dll  http://tools.openoffice.org/unowinreg_prebuild/680/unowinreg.dll

#
# Run Configure
#
date && echo "running configure"
autoconf
rc=$?
if [ $rc != 0 ] ; then
    echo "Exiting build - autoconf was unsucessful."
    exit $rc
fi

./configure --with-jdk-home=/usr/lib/jvm/java-6-sun  --with-system-python --enable-verbose
rc=$?
if [ $rc != 0 ] ; then
    echo "Exiting build - configure was unsucessful."
    exit $rc
fi

#
# Bootstrap
#
date && echo "running bootstrap"
./bootstrap
rc=$?
if [ $rc != 0 ] ; then
    echo "Exiting build - bootstrapping was unsuccessful."
    exit $rc
fi

#
# set environment
#
. ./LinuxX86Env.Set.sh

#
# Run build
#
date  && echo "running the build"
cd instsetoo_native && build --all --html -P2 -- -P2
date

