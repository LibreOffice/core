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

PRJ = ..$/..$/..

PRJNAME	= lingucomponent
TARGET	= MacOSXSpell
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE

.IF "$(GUIBASE)"=="aqua"

#----- Settings ---------------------------------------------------------

.INCLUDE : settings.mk

.IF "$(SYSTEM_HUNSPELL)" != "YES"
HUNSPELL_CFLAGS += -I$(SOLARINCDIR)$/hunspell -DHUNSPELL_STATIC
.ENDIF

CXXFLAGS += $(HUNSPELL_CFLAGS)
CFLAGSCXX += $(HUNSPELL_CFLAGS)
CFLAGSCC += $(HUNSPELL_CFLAGS)

# --- Files --------------------------------------------------------

CFLAGSCXX+=$(OBJCXXFLAGS)
CFLAGSCXX+=-I..$/..$/lingutil

EXCEPTIONSFILES=	\
        $(SLO)$/macspellimp.obj


SLOFILES=	\
        $(SLO)$/macreg.obj\
        $(SLO)$/macspellimp.obj


SHL1TARGET= $(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS= \
        $(CPPULIB) 	 \
        $(CPPUHELPERLIB) 	 \
        $(TOOLSLIB)		\
        $(SVLLIB)		\
        $(SALLIB)		\
        $(UCBHELPERLIB)	\
        $(UNOTOOLSLIB)	\
        $(LNGLIB)

SHL1STDLIBS+= -framework Cocoa

# build DLL
SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1IMPLIB=		i$(TARGET)
SHL1DEPN=		$(SHL1LIBS)
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

SHL1VERSIONMAP=$(SOLARENV)/src/component.map

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk

.ELSE
dummy:
    @echo " Nothing to build for GUIBASE=$(GUIBASE)"
.ENDIF
