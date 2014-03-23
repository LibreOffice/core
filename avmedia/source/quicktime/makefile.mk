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
PRJNAME=avmedia
TARGET=avmediaQuickTime

# the QuickTime API has been deprecated since OSX 10.5 and has been removed in the OSX SDK 10.7
.IF ("$(GUIBASE)" != "aqua") || ("${MACOSX_DEPLOYMENT_TARGET:s/.//}" >= "106")
dummy:
    @echo " Nothing to build for GUIBASE=$(GUIBASE) and OSX$(MACOSX_DEPLOYMENT_TARGET)"
.ELSE

# --- Settings ----------------------------------

.INCLUDE :  	settings.mk

.IF "$(verbose)"!="" || "$(VERBOSE)"!=""
CDEFS+= -DVERBOSE
.ENDIF

# --- Files ----------------------------------

CFLAGSCXX+=$(OBJCXXFLAGS)

SLOFILES= \
        $(SLO)$/quicktimeuno.obj  \
        $(SLO)$/framegrabber.obj        \
        $(SLO)$/manager.obj       \
        $(SLO)$/window.obj        \
        $(SLO)$/player.obj

EXCEPTIONSFILES= \
        $(SLO)$/framegrabber.obj        \
        $(SLO)$/quicktimeuno.obj

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS= \
             $(CPPULIB) \
             $(SALLIB)  \
             $(COMPHELPERLIB) \
             $(CPPUHELPERLIB) \
             $(TOOLSLIB) \
             $(VCLLIB) 

SHL1STDLIBS+= \
             -framework Cocoa \
             -framework QTKit \
             -framework QuickTime

# build DLL
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1IMPLIB=i$(TARGET)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

SHL1VERSIONMAP=$(SOLARENV)/src/component.map

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk

.ENDIF

ALLTAR : $(MISC)/avmediaQuickTime.component

$(MISC)/avmediaQuickTime.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt avmediaQuickTime.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt avmediaQuickTime.component

