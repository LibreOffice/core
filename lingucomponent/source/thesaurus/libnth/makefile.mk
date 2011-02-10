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
TARGET	= lnth
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE


.IF "$(MYTHESLIB)"==""
.IF "$(GUI)"=="UNX"
MYTHESLIB=-lmythes
.ENDIF # unx
.IF "$(GUI)"=="OS2"
MYTHESLIB=$(SLB)\libmythes.lib
.ENDIF # os2
.IF "$(GUI)"=="WNT"
MYTHESLIB=libmythes.lib
.ENDIF # wnt
.ENDIF

#----- Settings ---------------------------------------------------------

.INCLUDE : settings.mk

# --- Files --------------------------------------------------------

.IF "$(SYSTEM_HUNSPELL)" != "YES"
HUNSPELL_CFLAGS += -I$(SOLARINCDIR)$/hunspell -DHUNSPELL_STATIC
.ENDIF

.IF "$(SYSTEM_MYTHES)" != "YES"
CXXFLAGS += -I..$/mythes
CFLAGSCXX += -I..$/mythes
CFLAGSCC += -I..$/mythes
.ENDIF
CXXFLAGS += -I$(PRJ)$/source$/lingutil $(HUNSPELL_CFLAGS)
CFLAGSCXX += -I$(PRJ)$/source$/lingutil $(HUNSPELL_CFLAGS)
CFLAGSCC += -I$(PRJ)$/source$/lingutil $(HUNSPELL_CFLAGS)

EXCEPTIONSFILES=	\
        $(SLO)$/nthesimp.obj \
        $(SLO)$/nthesdta.obj

SLOFILES=	\
        $(SLO)$/nthesdta.obj\
        $(SLO)$/ntreg.obj\
        $(SLO)$/nthesimp.obj


SHL1TARGET= $(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS= \
        $(CPPULIB) 	 \
        $(CPPUHELPERLIB) 	 \
        $(TOOLSLIB)		\
        $(I18NISOLANGLIB)   \
        $(SVLLIB)		\
        $(SALLIB)		\
        $(UNOTOOLSLIB)	\
        $(LNGLIB) \
                $(ULINGULIB) \
        $(MYTHESLIB) \
        $(HUNSPELLLIB)

# build DLL
SHL1LIBS=       $(SLB)$/$(TARGET).lib $(SLB)$/libulingu.lib
SHL1IMPLIB=		i$(TARGET)
SHL1DEPN=		$(SHL1LIBS)
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

SHL1VERSIONMAP=$(SOLARENV)/src/component.map

# build DEF file
DEF1NAME	 =$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk

