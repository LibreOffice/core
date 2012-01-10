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

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

# BEGIN ----------------------------------------------------------------
# auto generated Target:testjob by codegen.pl

.IF "$(GUI)" == "WNT"
    CFLAGS+=/Ob1
.ENDIF

SHL1OBJS=  \
    $(SLO)$/osl_Thread.obj

SHL1TARGET= osl_Thread
SHL1STDLIBS= $(SALLIB) $(CPPUNITLIB) $(TESTSHL2LIB)

SHL1IMPLIB= i$(SHL1TARGET)

# SHL1DEF=    $(MISC)$/$(SHL1TARGET).def

DEF1NAME    =$(SHL1TARGET)

# DEF1EXPORTFILE= export.exp
SHL1VERSIONMAP = $(PRJ)$/qa$/export.map

# END ------------------------------------------------------------------

#.IF "$(GUI)" == "WNT"

SHL2OBJS=$(SLO)$/osl_process.obj
SHL2TARGET=osl_process
SHL2STDLIBS= $(SALLIB) $(CPPUNITLIB) $(TESTSHL2LIB)

SHL2IMPLIB=i$(SHL2TARGET)
SHL2DEF=$(MISC)$/$(SHL2TARGET).def
DEF2NAME=$(SHL2TARGET)
DEF2EXPORTFILE=export.exp

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

#------------------------------- All object files -------------------------------
# do this here, so we get right dependencies

.IF "$(GUI)" == "OS2"

SLOFILES=$(SHL2OBJS)

.ELSE

SLOFILES=$(SHL1OBJS) $(SHL2OBJS)

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE : _cppunit.mk
