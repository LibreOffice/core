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
TARGET=qa_module

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

# BEGIN ----------------------------------------------------------------

# --- test dll ------------------------------------------------------
SHL1TARGET     = Module_DLL
SHL1OBJS       = $(SLO)$/osl_Module_DLL.obj
SHL1STDLIBS    = $(SALLIB)
SHL1IMPLIB     = i$(SHL1TARGET)
SHL1DEF        = $(MISC)$/$(SHL1TARGET).def
DEF1NAME       = $(SHL1TARGET)
SHL1VERSIONMAP = export_dll.map


# --- main l ------------------------------------------------------
SHL2OBJS=  $(SLO)$/osl_Module.obj

SHL2TARGET= osl_Module
SHL2STDLIBS= $(SALLIB) $(CPPUNITLIB) $(TESTSHL2LIB)

.IF "$(GUI)" == "WNT"
SHL2STDLIBS+=i$(SHL2TARGET).lib
.ENDIF
.IF "$(GUI)" == "UNX"
APP3STDLIBS+=-l$(SHL2TARGET)
.ENDIF

SHL2DEPN= $(SHL1OBJS)
SHL2IMPLIB= i$(SHL2TARGET)
SHL2DEF=    $(MISC)$/$(SHL2TARGET).def

DEF2NAME    =$(SHL2TARGET)
SHL2VERSIONMAP= $(PRJ)$/qa$/export.map
# END ------------------------------------------------------------------

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE : _cppunit.mk

