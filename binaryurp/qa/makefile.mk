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



PRJ = ..
PRJNAME = binaryurp
TARGET = qa

ENABLE_EXCEPTIONS = TRUE

.INCLUDE: settings.mk

.IF "$(WITH_CPPUNIT)" != "YES" || "$(GUI)" == "OS2"

@all:
.IF "$(GUI)" == "OS2"
    @echo "Skipping, cppunit broken."
.ELIF "$(WITH_CPPUNIT)" != "YES"
    @echo "cppunit disabled. nothing do do."
.END

.ELSE

CFLAGSCXX += $(CPPUNIT_CFLAGS)

DLLPRE =

.IF "$(GUI)" != "OS2"
SLOFILES = $(SLO)/test-cache.obj $(SLO)/test-unmarshal.obj
.ENDIF

SHL1IMPLIB = i$(SHL1TARGET)
SHL1OBJS = $(SLO)/test-cache.obj
SHL1RPATH = NONE
SHL1STDLIBS = $(CPPUNITLIB) $(SALLIB)
.IF "$(GUI)" != "OS2"
SHL1TARGET = test-cache
.ELSE
SHL1TARGET = test-c
.ENDIF
SHL1VERSIONMAP = version.map
DEF1NAME = $(SHL1TARGET)

SHL2IMPLIB = i$(SHL2TARGET)
SHL2OBJS = \
    $(SLO)/test-unmarshal.obj \
    $(SLO)/binaryany.obj \
    $(SLO)/bridge.obj \
    $(SLO)/bridgefactory.obj \
    $(SLO)/currentcontext.obj \
    $(SLO)/incomingrequest.obj \
    $(SLO)/lessoperators.obj \
    $(SLO)/marshal.obj \
    $(SLO)/outgoingrequests.obj \
    $(SLO)/proxy.obj \
    $(SLO)/reader.obj \
    $(SLO)/unmarshal.obj \
    $(SLO)/writer.obj
SHL2RPATH = NONE
SHL2STDLIBS = \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(CPPUNITLIB) \
    $(SALHELPERLIB) \
    $(SALLIB)
.IF "$(GUI)" != "OS2"
SHL2TARGET = test-unmarshal
.ELSE
SHL2TARGET = test-u
.ENDIF
SHL2VERSIONMAP = version.map
DEF2NAME = $(SHL2TARGET)

.ENDIF # "$(GUI)" == "OS2"

.INCLUDE: target.mk
.INCLUDE: _cppunit.mk

