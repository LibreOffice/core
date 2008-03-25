#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.18 $
#
#   last change: $Author: obo $ $Date: 2008-03-25 16:49:16 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

PRJ = ..$/..$/..$/..

PRJNAME	= lingucomponent
TARGET	= hyphen
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE

.IF "$(GUI)"=="UNX"
.IF "$(SYSTEM_HYPH)" == "YES"
HNJLIB=$(HYPHEN_LIB)
.ELSE
HNJLIB=-lhnj
.ENDIF
.ELIF  "$(GUI)"=="OS2"
HNJLIB=$(SLB)\libhnj.lib
.ELSE
HNJLIB=libhnj.lib
.ENDIF

.IF "$(ULINGULIB)"==""
.IF "$(GUI)"=="UNX"
ULINGULIB=-lulingu
.ENDIF # unx
.IF "$(GUI)"=="OS2"
ULINGULIB=$(SLB)\libulingu.lib
.ENDIF # unx
.IF "$(GUI)"=="WNT"
ULINGULIB=libulingu.lib
.ENDIF # wnt
.ENDIF



#----- Settings ---------------------------------------------------------

.INCLUDE : settings.mk

# --- Files --------------------------------------------------------


CXXFLAGS += -I..$/libhnj -I..$/..$/..$/lingutil
CFLAGSCXX += -I..$/libhnj -I..$/..$/..$/lingutil
CFLAGSCC += -I..$/libhnj -I..$/..$/..$/lingutil

.IF "$(header)" == ""

EXCEPTIONSFILES=	\
        $(SLO)$/hprophelp.obj\
        $(SLO)$/hyphenimp.obj

SLOFILES=	\
        $(SLO)$/hprophelp.obj\
        $(SLO)$/hreg.obj\
        $(SLO)$/hyphenimp.obj


SHL1TARGET= $(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS= \
        $(CPPULIB) 	 \
        $(CPPUHELPERLIB) 	 \
        $(VOSLIB)		\
        $(I18NISOLANGLIB)   \
        $(TOOLSLIB)		\
        $(SVTOOLLIB)	\
        $(SVLLIB)		\
        $(VCLLIB)		\
        $(SALLIB)		\
        $(UCBHELPERLIB)	\
        $(UNOTOOLSLIB)	\
        $(LNGLIB) \
        $(HNJLIB)

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

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk

