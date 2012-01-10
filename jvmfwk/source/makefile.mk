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
PRJNAME = jvmfwk
FRAMEWORKLIB=jvmfwk
TARGET = $(FRAMEWORKLIB)
ENABLE_EXCEPTIONS = TRUE

.IF "$(OS)" != "WNT" && "$(GUI)"!="OS2"
UNIXVERSIONNAMES = UDK
.ENDIF # WNT

.INCLUDE: settings.mk

.IF "$(SYSTEM_LIBXML)" == "YES"
CFLAGS+=-DSYSTEM_LIBXML $(LIBXML_CFLAGS)
.ENDIF

UNOUCROUT = $(OUT)$/inc

SLOFILES = \
    $(SLO)$/framework.obj \
    $(SLO)$/libxmlutil.obj \
    $(SLO)$/fwkutil.obj \
    $(SLO)$/elements.obj \
    $(SLO)$/fwkbase.obj


#LIB1TARGET=$(SLB)$/$(FRAMEWORKLIB).lib

.IF "$(UNIXVERSIONNAMES)" == ""
SHL1TARGET = $(FRAMEWORKLIB)$(UDK_MAJOR)
.ELSE # UNIXVERSIONNAMES
SHL1TARGET = $(FRAMEWORKLIB)
.ENDIF # UNIXVERSIONNAMES

#SHL1TARGET=$(FRAMEWORKLIB)
SHL1DEPN=
SHL1IMPLIB = i$(FRAMEWORKLIB)
SHL1LIBS = $(SLB)$/$(TARGET).lib
SHL1STDLIBS = $(CPPUHELPERLIB) $(SALLIB) $(LIBXML2LIB)
SHL1RPATH = URELIB

.IF "$(OS)" == "WNT"
SHL1STDLIBS += $(ADVAPI32LIB)
.ENDIF # WNT

SHL1VERSIONMAP = framework.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME = $(SHL1TARGET)

.IF "$(GUI)"=="UNX"
RCFILE=$(BIN)$/jvmfwk3rc
.ELIF "$(GUI)"=="WNT" || "$(GUI)"=="OS2"
RCFILE=$(BIN)$/jvmfwk3.ini
.END


.INCLUDE: target.mk
$(RCFILE): jvmfwk3rc
    -$(COPY) $< $@

ALLTAR: \
    $(RCFILE)

