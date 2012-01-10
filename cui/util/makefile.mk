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



PRJ=..

PRJNAME=cui
TARGET=cui
GEN_HID=TRUE
GEN_HID_OTHER=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(OS)"=="IRIX"
LINKFLAGS+=-Wl,-LD_LAYOUT:lgot_buffer=30
.ENDIF

SHL1TARGET= cui$(DLLPOSTFIX)
SHL1VERSIONMAP= cui.map
SHL1IMPLIB=icui
SHL1DEPN=
SHL1DEF=        $(MISC)$/$(SHL1TARGET).def
DEF1NAME=       $(SHL1TARGET)

SHL1LIBS=	\
            $(SLB)$/options.lib  \
            $(SLB)$/customize.lib  \
            $(SLB)$/dialogs.lib  \
            $(SLB)$/tabpages.lib \
            $(SLB)$/factory.lib


SHL1STDLIBS= \
            $(EDITENGLIB) \
            $(SVXCORELIB) \
            $(SVXLIB) \
            $(AVMEDIALIB) \
            $(SFX2LIB) \
            $(BASEGFXLIB) \
            $(BASICLIB) \
            $(LNGLIB) \
            $(SVTOOLLIB) \
            $(TKLIB) \
            $(VCLLIB) \
            $(SVLLIB) \
            $(UNOTOOLSLIB) \
            $(TOOLSLIB) \
            $(I18NISOLANGLIB) \
            $(COMPHELPERLIB) \
            $(UCBHELPERLIB) \
            $(CPPUHELPERLIB)        \
            $(CPPULIB) \
            $(VOSLIB) \
            $(SALLIB) \
            $(JVMFWKLIB) \
            $(ICUUCLIB)

.IF "$(GUI)"=="WNT"
SHL1STDLIBS+= \
             $(SHLWAPILIB) \
             $(ADVAPI32LIB)
.ENDIF # WNT

# ------------------------------------------------------------------------------

# Resource files
SRSFILELIST=\
        $(SRS)$/options.srs \
        $(SRS)$/dialogs.srs \
        $(SRS)$/tabpages.srs \
        $(SRS)$/customize.srs \


RESLIB1NAME=cui
RESLIB1IMAGES=$(SOLARSRC)/$(RSCDEFIMG)/svx/res
RESLIB1SRSFILES= $(SRSFILELIST)

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

