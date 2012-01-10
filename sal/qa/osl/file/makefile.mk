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
TARGET=qa_osl_file

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

# --- BEGIN --------------------------------------------------------
SHL1OBJS=  \
    $(SLO)$/osl_File.obj
SHL1TARGET= osl_File
SHL1STDLIBS= $(SALLIB) $(CPPUNITLIB) $(TESTSHL2LIB)

SHL1IMPLIB= i$(SHL1TARGET)

DEF1NAME    =$(SHL1TARGET)
SHL1VERSIONMAP = $(PRJ)$/qa$/export.map

#-------------------------------------------------------------------

SHL2OBJS=$(SLO)$/test_cpy_wrt_file.obj
SHL2TARGET=tcwf
SHL2STDLIBS= $(SALLIB) $(CPPUNITLIB) $(TESTSHL2LIB)
SHL2IMPLIB=i$(SHL2TARGET)
SHL2DEF=$(MISC)$/$(SHL2TARGET).def
SHL2VERSIONMAP = $(PRJ)$/qa$/export.map
DEF2NAME    =$(SHL2TARGET)


# END --------------------------------------------------------------

# --- BEGIN --------------------------------------------------------
SHL3OBJS=  \
    $(SLO)$/osl_old_test_file.obj
SHL3TARGET= osl_old_test_file
SHL3STDLIBS= $(SALLIB) $(CPPUNITLIB) $(TESTSHL2LIB)

SHL3IMPLIB= i$(SHL3TARGET)

DEF3NAME    =$(SHL3TARGET)
SHL3VERSIONMAP = $(PRJ)$/qa$/export.map
# END --------------------------------------------------------------

#------------------------------- All object files -------------------------------
# do this here, so we get right dependencies
SLOFILES=$(SHL1OBJS)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE : _cppunit.mk
