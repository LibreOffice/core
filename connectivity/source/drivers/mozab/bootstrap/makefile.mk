#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************
CALL_CDECL=TRUE

PRJ=..$/..$/..$/..
PRJINC=..$/..$/..
PRJNAME=connectivity
TARGET=mozbootstrap

.IF  ("$(SYSTEM_MOZILLA)" == "YES" && "$(WITH_MOZILLA)" == "YES") || "$(WITH_MOZILLA)" == "NO" || "$(OS)" == "OS2"
all: 
    @echo "    Not building the mozbootstrap stuff in LibreOffice build"
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
