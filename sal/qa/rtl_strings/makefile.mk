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


PRJ=..$/..

PRJNAME=sal
TARGET=qa_rtl_strings

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

# BEGIN ----------------------------------------------------------------
# auto generated Target:FileBase by codegen.pl
SHL1OBJS=  \
    $(SLO)$/rtl_String_Utils.obj \
    $(SLO)$/rtl_OString.obj

SHL1TARGET= rtl_OString
SHL1STDLIBS= $(SALLIB) $(CPPUNITLIB) $(TESTSHL2LIB)

SHL1IMPLIB= i$(SHL1TARGET)
# SHL1DEF=    $(MISC)$/$(SHL1TARGET).def

DEF1NAME    =$(SHL1TARGET)
# DEF1EXPORTFILE= export.exp
SHL1VERSIONMAP = $(PRJ)$/qa$/export.map

# auto generated Target:FileBase
# END ------------------------------------------------------------------

# BEGIN ----------------------------------------------------------------
# auto generated Target:FileBase by codegen.pl
SHL2OBJS=  \
    $(SLO)$/rtl_String_Utils.obj \
    $(SLO)$/rtl_OUString.obj

SHL2TARGET= rtl_OUString
SHL2STDLIBS= $(SALLIB) $(CPPUNITLIB) $(TESTSHL2LIB)

SHL2IMPLIB= i$(SHL2TARGET)
# SHL2DEF=    $(MISC)$/$(SHL2TARGET).def

DEF2NAME    =$(SHL2TARGET)
# DEF2EXPORTFILE= export.exp
SHL2VERSIONMAP = $(PRJ)$/qa$/export.map

# auto generated Target:FileBase
# END ------------------------------------------------------------------

# BEGIN ----------------------------------------------------------------
# auto generated Target:FileBase by codegen.pl
SHL3OBJS=  \
    $(SLO)$/rtl_String_Utils.obj \
    $(SLO)$/rtl_OUStringBuffer.obj

SHL3TARGET= rtl_OUStringBuffer
SHL3STDLIBS= $(SALLIB) $(CPPUNITLIB) $(TESTSHL2LIB)

SHL3IMPLIB= i$(SHL3TARGET)
# SHL3DEF=    $(MISC)$/$(SHL3TARGET).def

DEF3NAME    =$(SHL3TARGET)
# DEF3EXPORTFILE= export.exp
SHL3VERSIONMAP = $(PRJ)$/qa$/export.map

# auto generated Target:FileBase
# END ------------------------------------------------------------------

# BEGIN ----------------------------------------------------------------
SHL4OBJS=  \
    $(SLO)$/rtl_old_teststrbuf.obj

SHL4TARGET= rtl_old_teststrbuf
SHL4STDLIBS= $(SALLIB) $(CPPUNITLIB) $(TESTSHL2LIB)

SHL4IMPLIB= i$(SHL4TARGET)
DEF4NAME    =$(SHL4TARGET)
SHL4VERSIONMAP = $(PRJ)$/qa$/export.map

# END ------------------------------------------------------------------

# BEGIN ----------------------------------------------------------------
SHL5OBJS=  \
    $(SLO)$/rtl_old_testowstring.obj

SHL5TARGET= rtl_old_testowstring
SHL5STDLIBS= $(SALLIB) $(CPPUNITLIB) $(TESTSHL2LIB)

SHL5IMPLIB= i$(SHL5TARGET)
DEF5NAME    =$(SHL5TARGET)
SHL5VERSIONMAP = $(PRJ)$/qa$/export.map

# END ------------------------------------------------------------------

# BEGIN ----------------------------------------------------------------
SHL6OBJS=  \
    $(SLO)$/rtl_old_testostring.obj

SHL6TARGET= rtl_old_testostring
SHL6STDLIBS= $(SALLIB) $(CPPUNITLIB) $(TESTSHL2LIB)

SHL6IMPLIB= i$(SHL6TARGET)
DEF6NAME    =$(SHL6TARGET)
SHL6VERSIONMAP = $(PRJ)$/qa$/export.map

# END ------------------------------------------------------------------

#------------------------------- All object files -------------------------------
# do this here, so we get right dependencies
SLOFILES=\
    $(SHL1OBJS) \
    $(SHL2OBJS) \
    $(SHL3OBJS) \
    $(SHL4OBJS) \
    $(SHL5OBJS) \
    $(SHL6OBJS)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE : _cppunit.mk


