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

PRJNAME=basctl
TARGET=basctl
#svx.hid generieren
GEN_HID=TRUE
GEN_HID_OTHER=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Allgemein ----------------------------------------------------

# --- Basctl - DLL ----------

.IF "$(header)" == ""

HELPIDFILES=    ..$/inc$/helpid.hrc

SHL1TARGET= basctl$(DLLPOSTFIX)
SHL1IMPLIB= basctl
SHL1BASE  = 0x1d800000
SHL1STDLIBS= \
            $(BASEGFXLIB) \
            $(EDITENGLIB) \
            $(SVXCORELIB) \
            $(SVXLIB) \
            $(SFX2LIB) \
            $(FWELIB) \
            $(BASICLIB) \
            $(SVTOOLLIB) \
            $(TKLIB) \
            $(VCLLIB) \
            $(SVLLIB) \
            $(SOTLIB) \
            $(UNOTOOLSLIB) \
            $(TOOLSLIB) \
            $(XMLSCRIPTLIB) \
            $(COMPHELPERLIB) \
            $(CPPUHELPERLIB) \
            $(UCBHELPERLIB) \
            $(CPPULIB) \
            $(SALLIB)

.IF "$(GUI)"=="WNT"
SHL1STDLIBS+= $(SHELLLIB)
.ENDIF # WNT


SHL1LIBS=       $(SLB)$/basctl.lib
SHL1DEF=        $(MISC)$/$(SHL1TARGET).def

SHL1VERSIONMAP=basctl.map

DEF1NAME        =$(SHL1TARGET)

LIB1TARGET      =$(SLB)$/basctl.lib
LIB1FILES       = \
            $(LIBPRE) $(SLB)$/basicide.lib		\
            $(LIBPRE) $(SLB)$/dlged.lib			\
            $(LIBPRE) $(SLB)$/accessibility.lib

SRSFILELIST=\
                $(SRS)$/basicide.srs \
                $(SRS)$/dlged.srs

RESLIB1NAME=basctl
RESLIB1IMAGES =$(PRJ)$/res
RESLIB1SRSFILES= $(SRSFILELIST)

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

.ENDIF # "$(header)" == ""


ALLTAR : $(MISC)/basctl.component

$(MISC)/basctl.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        basctl.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt basctl.component
