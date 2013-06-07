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

# enable file locking
SAL_ENABLE_FILE_LOCKING=1
export SAL_ENABLE_FILE_LOCKING

# resolve installation directory
sd_cwd=`pwd`
sd_res=$0
while [ -h "$sd_res" ] ; do
    cd "`dirname "$sd_res"`"
    sd_basename=`basename "$sd_res"`
    sd_res=`ls -l "$sd_basename" | sed "s/.*$sd_basename -> //g"`
done
cd "`dirname "$sd_res"`"
sd_prog=`pwd`
cd "$sd_cwd"

#collect all bootstrap variables specified on the command line
#so that they can be passed as arguments to javaldx later on
#Recognize the "sync" option. sync must be applied without any other
#options except bootstrap variables or the verbose option
for arg in $@
do
  case "$arg" in
       -env:*) BOOTSTRAPVARS=$BOOTSTRAPVARS" ""$arg";;
       sync) OPTSYNC=true;;
       -v) VERBOSE=true;;
       --verbose) VERBOSE=true;;
       *) OPTOTHER=$arg;;
  esac
done

if [ "$OPTSYNC" = "true" ] && [ -z "$OPTOTHER" ]
then
    JVMFWKPARAMS='-env:UNO_JAVA_JFW_INSTALL_DATA=$OOO_BASE_DIR/share/config/javasettingsunopkginstall.xml -env:JFW_PLUGIN_DO_NOT_CHECK_ACCESSIBILITY=1'
fi

# extend the ld_library_path for java: javaldx checks the sofficerc for us
if [ -x "$sd_prog/../basis-link/ure-link/bin/javaldx" ] ; then
    my_path=`"$sd_prog/javaldx" $BOOTSTRAPVARS $JVMFWKPARAMS \
        "-env:INIFILENAME=vnd.sun.star.pathname:$sd_prog/redirectrc"`
    if [ -n "$my_path" ] ; then
        LD_LIBRARY_PATH=$my_path${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
        export LD_LIBRARY_PATH
    fi
fi

unset XENVIRONMENT

# uncomment line below to disable anti aliasing of fonts
# SAL_ANTIALIAS_DISABLE=true; export SAL_ANTIALIAS_DISABLE

# uncomment line below if you encounter problems starting soffice on your system
# SAL_NO_XINITTHREADS=true; export SAL_NO_XINITTHREADS

# execute binary
exec "$sd_prog/unopkg.bin" "$@"  "$JVMFWKPARAMS" \
    "-env:INIFILENAME=vnd.sun.star.pathname:$sd_prog/redirectrc"

