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


CALL_CDECL=TRUE

PRJ=..$/..$/..$/..
PRJINC=..$/..$/..
PRJNAME=connectivity
TARGET=mozbootstrap

.IF "$(OS)" == "OS2"
all: 
    @echo "    Not building under OS/2"
.ENDIF

# --- Settings ----------------------------------

USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

# --- begin of mozilla specific stuff
MOZ_LIB=$(SOLARVERSION)$/$(INPATH)$/lib$(UPDMINOREXT)
MOZ_INC=$(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/mozilla
# --- end of mozilla specific stuff

.INCLUDE : settings.mk

.INCLUDE :  $(PRJ)$/version.mk

.INCLUDE : ../makefile_mozab.mk

INCPRE += -I../mozillasrc

# --- Files -------------------------------------

SLOFILES += \
    $(SLO)$/MNSINIParser.obj \
    $(SLO)$/MNSProfileDiscover.obj \
    $(SLO)$/MMozillaBootstrap.obj \
    $(SLO)$/MNSFolders.obj

.IF ( "$(SYSTEM_MOZILLA)" == "YES" && "$(WITH_MOZILLA)" == "YES" ) || ( "$(WITH_MOZILLA)" == "NO" ) || ( "$(OS)" == "MACOSX" )
CDEFS+=-DMINIMAL_PROFILEDISCOVER

SHL1TARGET=$(TARGET)
SHL1VERSIONMAP=$(SOLARENV)/src/component.map
SHL1OBJS=$(SLOFILES)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

SHL1STDLIBS=\
        $(CPPULIB)       \
        $(CPPUHELPERLIB) \
        $(SALLIB)        \
        $(COMPHELPERLIB)

ALLTAR : $(MISC)/mozbootstrap.component

$(MISC)/mozbootstrap.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt mozbootstrap.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt mozbootstrap.component

.ELSE
SLOFILES += \
    $(SLO)$/MNSInit.obj			            \
    $(SLO)$/MNSProfileManager.obj	\
    $(SLO)$/MNSRunnable.obj	\
    $(SLO)$/MNSProfile.obj					\
    $(SLO)$/MNSProfileDirServiceProvider.obj
.ENDIF

# --- Targets ----------------------------------

.INCLUDE : target.mk
