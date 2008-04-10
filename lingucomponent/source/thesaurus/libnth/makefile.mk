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
# $Revision: 1.14 $
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

.IF "$(ULINGULIB)"==""
.IF "$(GUI)"=="UNX"
ULINGULIB=-lulingu
.ENDIF # unx
.IF "$(GUI)"=="OS2"
ULINGULIB=$(SLB)\libulingu.lib
.ENDIF # os2
.IF "$(GUI)"=="WNT"
ULINGULIB=libulingu.lib
.ENDIF # wnt
.ENDIF


#----- Settings ---------------------------------------------------------

.INCLUDE : settings.mk

# --- Files --------------------------------------------------------

.IF "$(SYSTEM_HUNSPELL)" == "YES" && "$(SYSTEM_MYTHES)" == "YES"
CXXFLAGS += $(HUNSPELL_CFLAGS)
CFLAGSCXX += $(HUNSPELL_CFLAGS)
CFLAGSCC += $(HUNSPELL_CFLAGS)
.ENDIF
.IF "$(SYSTEM_HUNSPELL)" == "YES" && "$(SYSTEM_MYTHES)" != "YES"
CXXFLAGS += -I..$/mythes $(HUNSPELL_CFLAGS)
CFLAGSCXX += -I..$/mythes $(HUNSPELL_CFLAGS)
CFLAGSCC += -I..$/mythes $(HUNSPELL_CFLAGS)
.ENDIF
.IF "$(SYSTEM_HUNSPELL)" != "YES" && "$(SYSTEM_MYTHES)" == "YES"
CXXFLAGS += -I..$/..$/lingutil
CFLAGSCXX += -I..$/..$/lingutil
CFLAGSCC += -I..$/..$/lingutil
.ENDIF
.IF "$(SYSTEM_HUNSPELL)" != "YES" && "$(SYSTEM_MYTHES)" != "YES"
CXXFLAGS += -I..$/mythes -I..$/..$/lingutil
CFLAGSCXX += -I..$/mythes -I..$/..$/lingutil
CFLAGSCC += -I..$/mythes -I..$/..$/lingutil
.ENDIF

EXCEPTIONSFILES=	\
        $(SLO)$/ntprophelp.obj\
        $(SLO)$/nthesimp.obj

SLOFILES=	\
        $(SLO)$/nthesdta.obj\
        $(SLO)$/ntprophelp.obj\
        $(SLO)$/ntreg.obj\
        $(SLO)$/nthesimp.obj


SHL1TARGET= $(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS= \
        $(CPPULIB) 	 \
        $(CPPUHELPERLIB) 	 \
        $(VOSLIB)		\
        $(TOOLSLIB)		\
        $(I18NISOLANGLIB)   \
        $(SVTOOLLIB)	\
        $(SVLLIB)		\
        $(VCLLIB)		\
        $(SALLIB)		\
        $(UCBHELPERLIB)	\
        $(UNOTOOLSLIB)	\
        $(LNGLIB) \
                $(MYTHESLIB)

.IF "$(SYSTEM_HUNSPELL)" != "YES"
SHL1STDLIBS+=   $(ULINGULIB)
.ELSE
SHL1STDLIBS+=   $(HUNSPELL_LIBS)
.ENDIF
# build DLL
SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1IMPLIB=		i$(TARGET)
SHL1DEPN=		$(SHL1LIBS)
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

SHL1VERSIONMAP= $(TARGET).map

# build DEF file
DEF1NAME	 =$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk

