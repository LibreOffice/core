#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.13 $
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

SHL1VERSIONMAP=exports.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

SHL1STDLIBS= \
    $(VCLLIB)			\
    $(SVLLIB)           \
    $(SVTOOLLIB)        \
    $(COMPHELPERLIB)    \
    $(UNOTOOLSLIB)		\
    $(TOOLSLIB)			\
    $(VOSLIB)			\
    $(CPPUHELPERLIB)	\
    $(CPPULIB)			\
    $(SALLIB)           \
    $(SFXLIB)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

$(SLO)$/splash.obj : $(INCCOM)$/introbmpnames.hxx

.INCLUDE .IGNORE : $(MISC)$/intro_bmp_names.mk

.IF "$(INTO_BITMAPS:f)"!="$(LASTTIME_INTRO_BITMAPS)"
DO_PHONY=.PHONY
.ENDIF			# "$(INTRO_BITMAPS:f)"!="$(LASTTIME_INTRO_BITMAPS)"

$(INCCOM)$/introbmpnames.hxx $(DO_PHONY):
    echo const char INTRO_BITMAP_STRINGLIST[]=$(EMQ)"$(INTRO_BITMAPS:f:t",")$(EMQ)"$(EMQ); > $@
    echo LASTTIME_INTRO_BITMAPS=$(INTRO_BITMAPS:f) > $(MISC)$/intro_bmp_names.mk
