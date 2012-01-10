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



PRJ=../..
PRJNAME=sax
TARGET=qa_cppunit

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(WITH_CPPUNIT)" != "YES" || "$(GUI)" == "OS2"

@all:
.IF "$(GUI)" == "OS2"
    @echo "Skipping, cppunit broken."
.ELIF "$(WITH_CPPUNIT)" != "YES"
    @echo "cppunit disabled. nothing do do."
.END

.ELSE # "$(WITH_CPPUNIT)" != "YES" || "$(GUI)" == "OS2"

#building with stlport, but cppunit was not built with stlport
.IF "$(USE_SYSTEM_STL)"!="YES"
.IF "$(SYSTEM_CPPUNIT)"=="YES"
CFLAGSCXX+=-DADAPT_EXT_STL
.ENDIF
.ENDIF

CFLAGSCXX += $(CPPUNIT_CFLAGS)
DLLPRE = # no leading "lib" on .so files

# --- Libs ---------------------------------------------------------

SHL1OBJS=  \
    $(SLO)/test_converter.obj \


SHL1STDLIBS= \
     $(SAXLIB) \
     $(SALLIB) \
     $(CPPUNITLIB) \


SHL1TARGET= test_converter
SHL1RPATH = NONE
SHL1IMPLIB= i$(SHL1TARGET)
# SHL1DEF= $(MISC)/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
# DEF1EXPORTFILE= export.exp
SHL1VERSIONMAP= version.map

# --- All object files ---------------------------------------------

SLOFILES= \
    $(SHL1OBJS) \


# --- Targets ------------------------------------------------------

.ENDIF # "$(WITH_CPPUNIT)" != "YES" || "$(GUI)" == "OS2"

.INCLUDE :  target.mk
.INCLUDE : _cppunit.mk

