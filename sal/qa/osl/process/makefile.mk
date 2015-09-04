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

PRJNAME=sal
TARGET=qa_osl_process

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(ENABLE_UNIT_TESTS)" != "YES"
all:
    @echo unit tests are disabled. Nothing to do.

.ELSE


CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

# BEGIN ----------------------------------------------------------------
# auto generated Target:testjob by codegen.pl

.IF "$(GUI)" == "WNT"
    CFLAGS+=/Ob1
.ENDIF

APP1OBJS=  \
    $(SLO)$/osl_Thread.obj

APP1TARGET= osl_Thread
APP1STDLIBS= $(SALLIB) $(GTESTLIB) $(TESTSHL2LIB)
APP1RPATH = NONE
APP1TEST = enabled

# END ------------------------------------------------------------------

#.IF "$(GUI)" == "WNT"

APP2OBJS=$(SLO)$/osl_process.obj
APP2TARGET=osl_process
APP2STDLIBS= $(SALLIB) $(GTESTLIB) $(TESTSHL2LIB)
APP2RPATH = NONE
APP2TEST = enabled

# END ------------------------------------------------------------------

OBJ3FILES=$(OBJ)$/osl_process_child.obj
APP3TARGET=osl_process_child
APP3OBJS=$(OBJ3FILES)

# .IF "$(GUI)" == "UNX"
# APP3STDLIBS=$(LB)$/libsal.so
# .ENDIF
# .IF "$(GUI)" == "WNT"
# APP3STDLIBS=$(KERNEL32LIB) $(LB)$/isal.lib
# .ENDIF
APP3STDLIBS=$(SALLIB)

#.ENDIF # "$(GUI)" == "WNT"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

.ENDIF # "$(ENABLE_UNIT_TESTS)" != "YES"
