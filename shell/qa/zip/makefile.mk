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

PRJNAME=shell
TARGET=qa_zip
ENABLE_EXCEPTIONS=TRUE
#USE_STLP_DEBUG= 
# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(WITH_CPPUNIT)" != "YES" || "$(GUI)" == "OS2"

@all:
.IF "$(GUI)" == "OS2"
    @echo "Skipping, cppunit broken."
.ELIF "$(WITH_CPPUNIT)" != "YES"
    @echo "cppunit disabled. nothing do do."
.END

.ELSE

CFLAGSCXX += $(CPPUNIT_CFLAGS)

SHL1OBJS = $(SLOFILES)
SHL1RPATH = NONE
SHL1STDLIBS = $(SALLIB) $(CPPUNITLIB)  
SHL1LIBS = $(SLB)$/..$/lib$/iqa_zipimpl.lib
SHL1TARGET = $(TARGET)
SHL1VERSIONMAP = $(PRJ)/qa/zip/export.map
DEF1NAME=$(SHL1TARGET)
SLOFILES=$(SLO)$/ziptest.obj  

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE: _cppunit.mk

