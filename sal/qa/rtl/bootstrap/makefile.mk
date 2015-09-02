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



PRJ=..$/..$/..
INCPRE+= $(PRJ)$/qa$/inc

PRJNAME=sal
TARGET=rtl_bootstrap

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(ENABLE_UNIT_TESTS)" != "YES"
all:
    @echo unit tests are disabled. Nothing to do.

.ELSE

.IF "$(GUI)"=="WNT"
#BOOTSTRAPSCRIPT=bootstrap.bat
BOOTSTRAPINI=testshl2.ini
MY_SCRIPTCAT=cat
.ELSE
#BOOTSTRAPSCRIPT=bootstrap
BOOTSTRAPINI=testshl2rc
MY_SCRIPTCAT=tr -d "\015" <
.ENDIF

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

# BEGIN ----------------------------------------------------------------
# auto generated Target:joblist by codegen.pl
APP1OBJS=  \
    $(SLO)$/rtl_Bootstrap.obj

APP1TARGET= rtl_Bootstrap
APP1STDLIBS= $(SALLIB) $(GTESTLIB) $(TESTSHL2LIB)
APP1RPATH = NONE
APP1TEST = enabled
# END ------------------------------------------------------------------

OBJ2FILES=$(OBJ)$/bootstrap_process.obj
APP2TARGET=bootstrap_process
APP2OBJS=$(OBJ2FILES)

# .IF "$(GUI)" == "UNX"
# APP2STDLIBS=$(LB)$/libsal.so
# .ENDIF
# .IF "$(GUI)" == "WNT"
# APP2STDLIBS=$(KERNEL32LIB) $(LB)$/isal.lib
# .ENDIF
APP2STDLIBS=$(SALLIB)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

.ENDIF # "$(ENABLE_UNIT_TESTS)" != "YES"