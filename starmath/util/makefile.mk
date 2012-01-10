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

PRJNAME=starmath
TARGET=smath3
LIBTARGET=NO
GEN_HID=TRUE
GEN_HID_OTHER=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

RESLIB1NAME=sm
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES=\
    $(SRS)$/smres.srs 

SHL1TARGET= sm$(DLLPOSTFIX)
SHL1IMPLIB= smimp

SHL1VERSIONMAP=$(SOLARENV)/src/component.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=		$(SHL1TARGET)

SHL1STDLIBS= \
            $(EDITENGLIB) \
            $(SVXCORELIB) \
            $(SVXLIB) \
            $(SFX2LIB) \
            $(XMLOFFLIB) \
            $(SVTOOLLIB) \
            $(TKLIB) \
            $(VCLLIB) \
            $(SVLLIB)	\
            $(I18NPAPERLIB)     \
            $(SOTLIB) \
            $(UNOTOOLSLIB) \
            $(TOOLSLIB) \
            $(COMPHELPERLIB) \
            $(CPPUHELPERLIB) \
            $(CPPULIB) \
            $(SALLIB)

SHL2TARGET= smd$(DLLPOSTFIX)
SHL2IMPLIB= smdimp
SHL2VERSIONMAP=$(SOLARENV)/src/component.map
SHL2DEF=$(MISC)$/$(SHL2TARGET).def
DEF2NAME=		$(SHL2TARGET)

SHL2STDLIBS= \
            $(SFX2LIB) \
            $(SVTOOLLIB) \
            $(SVLLIB) \
            $(VCLLIB) \
            $(SOTLIB) \
            $(TOOLSLIB) \
            $(UCBHELPERLIB) \
            $(CPPUHELPERLIB) \
            $(CPPULIB) \
            $(SALLIB)

SHL2DEPN=	makefile.mk
SHL2VERSIONMAP=$(SOLARENV)/src/component.map
SHL2OBJS=   $(SLO)$/smdetect.obj \
            $(SLO)$/detreg.obj \
            $(SLO)$/eqnolefilehdr.obj

SHL1DEPN=	makefile.mk
SHL1LIBS=   $(SLB)$/starmath.lib

.IF "$(GUI)" != "UNX"
.IF "$(GUI)$(COM)" != "WNTGCC"
SHL1OBJS=   $(SLO)$/smdll.obj
.ENDIF
.ENDIF # ! UNX

.IF "$(GUI)" == "WNT"
SHL1RES=	$(RCTARGET)
.ENDIF # WNT

# --- Targets -------------------------------------------------------------

.INCLUDE :  target.mk


ALLTAR : $(MISC)/sm.component $(MISC)/smd.component

$(MISC)/sm.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        sm.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt sm.component

$(MISC)/smd.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        smd.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL2TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt smd.component
