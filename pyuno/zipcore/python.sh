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

# resolve installation directory
sd_cwd="`pwd`"
if [ -h "$0" ] ; then
    sd_basename=`basename "$0"`
     sd_script=`ls -l "$0" | sed "s/.*${sd_basename} -> //g"`
    cd "`dirname "$0"`"
    cd "`dirname "$sd_script"`"
else
    cd "`dirname "$0"`"
fi
sd_prog=`pwd`
cd "$sd_cwd"

# Set PATH so that crash_report is found:
PATH=$sd_prog${PATH+:$PATH}
export PATH

# Set LD_LIBRARY_PATH so that "import pyuno" finds libpyuno.so:
LD_LIBRARY_PATH=$sd_prog:${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
export LD_LIBRARY_PATH

# Set UNO_PATH so that "officehelper.bootstrap()" can find soffice executable:
: ${UNO_PATH=$sd_prog}
export UNO_PATH

# Set URE_BOOTSTRAP so that "uno.getComponentContext()" bootstraps a complete
# OOo UNO environment:
: ${URE_BOOTSTRAP=vnd.sun.star.pathname:$sd_prog/fundamentalrc}
export URE_BOOTSTRAP

PYTHONPATH=$sd_prog:$sd_prog/python-core-%%PYVERSION%%/lib:$sd_prog/python-core-%%PYVERSION%%/lib/lib-dynload:$sd_prog/python-core-%%PYVERSION%%/lib/lib-tk:$sd_prog/python-core-%%PYVERSION%%/lib/site-packages${PYTHONPATH+:$PYTHONPATH}
export PYTHONPATH
PYTHONHOME=$sd_prog/python-core-%%PYVERSION%%
export PYTHONHOME

# execute binary
exec "$sd_prog/python.bin" "$@"
