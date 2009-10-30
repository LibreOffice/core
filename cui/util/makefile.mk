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
# $Revision: 1.61 $
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

PRJ=..

PRJNAME=cui
TARGET=cui
GEN_HID=TRUE
GEN_HID_OTHER=TRUE

USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(OS)"=="IRIX"
LINKFLAGS+=-Wl,-LD_LAYOUT:lgot_buffer=30
.ENDIF

SHL1TARGET= cui$(DLLPOSTFIX)
SHL1VERSIONMAP= cui.map
SHL1IMPLIB=icui

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEFNAME=	$(SHL1TARGET)

SHL1LIBS=	\
            $(SLB)$/options.lib  \
            $(SLB)$/customize.lib  \
            $(SLB)$/dialogs.lib  \
            $(SLB)$/tabpages.lib \
            $(SLB)$/factory.lib


SHL1STDLIBS= \
            $(SVXCORELIB) \
            $(SVXLIB) \
            $(AVMEDIALIB) \
            $(SFX2LIB) \
            $(GOODIESLIB) \
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

