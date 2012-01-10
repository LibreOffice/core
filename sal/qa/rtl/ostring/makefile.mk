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
TARGET=qa_rtl_ostring2

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

CFLAGSCXX += $(CPPUNIT_CFLAGS)

# BEGIN ----------------------------------------------------------------
# auto generated Target:joblist by codegen.pl
SHL1OBJS=  \
    $(SLO)$/rtl_OString2.obj

SHL1TARGET= rtl_OString2
SHL1STDLIBS= $(SALLIB) $(CPPUNITLIB) $(TESTSHL2LIB)

SHL1IMPLIB= i$(SHL1TARGET)
# SHL1DEF=    $(MISC)$/$(SHL1TARGET).def

DEF1NAME    =$(SHL1TARGET)
# DEF2EXPORTFILE= export.exp
SHL1VERSIONMAP= $(PRJ)$/qa$/export.map
# auto generated Target:joblist
# END ------------------------------------------------------------------

# BEGIN ----------------------------------------------------------------
SHL2OBJS=  \
    $(SLO)$/rtl_str.obj

SHL2TARGET= rtl_str
SHL2STDLIBS= $(SALLIB) $(CPPUNITLIB) $(TESTSHL2LIB)

SHL2IMPLIB= i$(SHL2TARGET)
DEF2NAME    =$(SHL2TARGET)
SHL2VERSIONMAP= $(PRJ)$/qa$/export.map
# END ------------------------------------------------------------------

# BEGIN ----------------------------------------------------------------
SHL3OBJS=  \
    $(SLO)$/rtl_string.obj

SHL3TARGET= rtl_string
SHL3STDLIBS= $(SALLIB) $(CPPUNITLIB) $(TESTSHL2LIB)

SHL3IMPLIB= i$(SHL3TARGET)
DEF3NAME    =$(SHL3TARGET)
SHL3VERSIONMAP= $(PRJ)$/qa$/export.map
# END ------------------------------------------------------------------
#------------------------------- All object files -------------------------------
# do this here, so we get right dependencies
# SLOFILES=$(SHL1OBJS)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE : _cppunit.mk

