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

PRJNAME=desktop
TARGET=spl
LIBTARGET=NO

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

SLOFILES =	$(SLO)$/splash.obj \
            $(SLO)$/firststart.obj \
            $(SLO)$/services_spl.obj

SHL1DEPN=   makefile.mk
SHL1OBJS=   $(SLOFILES) \
            $(SLO)$/pages.obj \
            $(SLO)$/wizard.obj \
            $(SLO)$/migration.obj \
            $(SLO)$/cfgfilter.obj


SHL1TARGET=$(TARGET)$(DLLPOSTFIX)
SHL1IMPLIB=i$(TARGET)

SHL1VERSIONMAP=$(SOLARENV)/src/component.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

SHL1STDLIBS= \
    $(VCLLIB)			\
    $(SVLLIB)           \
    $(SVTOOLLIB)        \
    $(COMPHELPERLIB)    \
    $(UNOTOOLSLIB)		\
    $(TOOLSLIB)			\
        $(UCBHELPERLIB)        \
    $(CPPUHELPERLIB)	\
    $(CPPULIB)			\
    $(VOSLIB)           \
    $(SALLIB)           \
    $(SFXLIB)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

$(SLO)$/splash.obj : $(INCCOM)$/introbmpnames.hxx

.INCLUDE .IGNORE : $(MISC)$/intro_bmp_names.mk

.IF "$(INTRO_BITMAPS:f)"!="$(LASTTIME_INTRO_BITMAPS)"
DO_PHONY=.PHONY
.ENDIF			# "$(INTRO_BITMAPS:f)"!="$(LASTTIME_INTRO_BITMAPS)"

$(INCCOM)$/introbmpnames.hxx $(DO_PHONY):
    echo const char INTRO_BITMAP_STRINGLIST[]=$(EMQ)"$(INTRO_BITMAPS:f:t",")$(EMQ)"$(EMQ); > $@
    echo LASTTIME_INTRO_BITMAPS=$(INTRO_BITMAPS:f) > $(MISC)$/intro_bmp_names.mk

ALLTAR : $(MISC)/spl.component

$(MISC)/spl.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        spl.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt spl.component
