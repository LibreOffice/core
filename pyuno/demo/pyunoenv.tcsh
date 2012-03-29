# *************************************************************
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
# *************************************************************

# the path to the office installation (e.g. /home/joe/OpenOffice.org1.1Beta)
setenv OOOHOME /src4/OpenOffice.org1.1Beta2

# don't modify anything beyond these lines
#---------------------------------------------
setenv PYTHONHOME $OOOHOME/program/python

if( ! $?LD_LIBRARY_PATH  ) then
    setenv LD_LIBRARY_PATH
endif

if(! $?PYTHONPATH ) then
    setenv PYTHONPATH
endif

if( ! $?LD_LIBRARY_PATH ) then
setenv LD_LIBRARY_PATH
endif

if( "$PYTHONPATH" != "" ) then
	setenv PYTHONPATH $OOOHOME/program:$OOOHOME/program/pydemo:$OOOHOME/program/python/lib:$PYTHONPATH
else
	setenv PYTHONPATH $OOOHOME/program:$OOOHOME/program/pydemo:$OOOHOME/program/python/lib
endif
	
setenv LD_LIBRARY_PATH $OOOHOME/program:$LD_LIBRARY_PATH

if( $?PYTHONHOME ) then
setenv PATH $PYTHONHOME/bin:$PATH
endif
