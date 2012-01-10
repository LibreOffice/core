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



#
# Mac OS X/Intel specific defines
#

PROCESSOR_DEFINES=-DX86

DLLPOSTFIX=

# special for SO build environment
.IF "$(SYSBASE)"!=""
.IF "$(EXTRA_CFLAGS)"!=""
CPP:=gcc -E $(EXTRA_CFLAGS)
CXXCPP*:=g++ -E $(EXTRA_CFLAGS)
.EXPORT : CPP CXXCPP
.ENDIF # "$(EXTRA_CFLAGS)"!=""
.ENDIF # "$(SYSBASE)"!=""

# flags to enable build with symbols; required by crashdump feature
.IF "$(ENABLE_SYMBOLS)"=="SMALL"
CFLAGSENABLESYMBOLS=-g1
.ELSE
CFLAGSENABLESYMBOLS=-g
.ENDIF

# Include generic Mac OS X makefile
.INCLUDE : unxmacx.mk
