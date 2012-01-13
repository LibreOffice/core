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



PRJ = ../..
PRJNAME = test
TARGET = cpp

ENABLE_EXCEPTIONS = TRUE
VISIBILITY_HIDDEN = TRUE

.INCLUDE: settings.mk

.IF "$(WITH_CPPUNIT)" != "YES" || "$(GUI)" == "OS2"

@all:
.IF "$(GUI)" == "OS2"
    @echo "Skipping, cppunit broken."
.ELIF "$(WITH_CPPUNIT)" != "YES"
    @echo "cppunit disabled. nothing do do."
.END

.ELSE

CDEFS += -DOOO_DLLIMPLEMENTATION_TEST

CFLAGSCXX += $(CPPUNIT_CFLAGS)
.IF "$(USE_SYSTEM_STL)" != "YES" && "$(SYSTEM_CPPUNIT)" == "YES"
CFLAGSCXX += -DADAPT_EXT_STL
.END

SLOFILES = \
    $(SLO)/getargument.obj \
    $(SLO)/gettestargument.obj \
    $(SLO)/officeconnection.obj \
    $(SLO)/toabsolutefileurl.obj \
    $(SLO)/uniquepipename.obj

SHL1IMPLIB = i$(SHL1TARGET)
SHL1OBJS = $(SLOFILES)
SHL1RPATH = NONE
SHL1STDLIBS = \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(CPPUNITLIB) \
    $(SALLIB)
SHL1TARGET = test
SHL1USE_EXPORTS = name
DEF1NAME = $(SHL1TARGET)

.ENDIF

.INCLUDE: target.mk
