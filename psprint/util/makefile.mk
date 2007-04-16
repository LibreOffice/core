#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.17 $
#
#   last change: $Author: ihi $ $Date: 2007-04-16 14:17:35 $
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

PRJ=..

PRJNAME=psprint
TARGET=psp

# --- Settings -----------------------------------------------------------

.INCLUDE :	settings.mk

# --- Allgemein ----------------------------------------------------------

.IF "$(GUIBASE)"=="aqua"

dummy:
    @echo "Nothing to build for GUIBASE $(GUIBASE)"

.ELSE		# "$(GUIBASE)"=="aqua"

.IF "$(OS)"=="WNT"
.IF "$(COM)"=="GCC"

LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1FILES=	$(SLB)$/fontsubset.lib

SHL1TARGET= psp$(UPD)$(DLLPOSTFIX)
SHL1IMPLIB= ipsp
SHL1LIBS=$(LIB1FILES)

SHL1STDLIBS=$(SALLIB)

SHL1VERSIONMAP=libpsp_mingw.map

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

# --- Def-File ---

DEF1NAME=	$(SHL1TARGET)
DEF1DES=PSPrint

.ELSE
LIB1TARGET= $(SLB)$/a$(TARGET).lib
LIB1FILES=	$(SLB)$/fontsubset.lib
.ENDIF
.ELSE

LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1FILES=	$(SLB)$/fontman.lib			\
            $(SLB)$/helper.lib			\
            $(SLB)$/printer.lib			\
            $(SLB)$/fontsubset.lib 		\
            $(SLB)$/gfx.lib

SHL1TARGET= psp$(UPD)$(DLLPOSTFIX)
SHL1IMPLIB= ipsp
SHL1LIBS=$(LIB1FILES)

.IF "$(OS)"=="SOLARIS" && "$(COM)"!="GCC"
SHL1VERSIONMAP=libpsp_solaris.map
.ELSE   # should work for all gcc/binutils based linker (ld)
SHL1VERSIONMAP=libpsp_linux.map
.ENDIF

SHL1STDLIBS=$(UNOTOOLSLIB)		\
            $(I18NISOLANGLIB) 	\
            $(TOOLSLIB) 		\
            $(SALLIB)			\
            -lX11
.IF "$(SOLAR_JAVA)"!=""
SHL1STDLIBS+=$(JVMFWKLIB)
.ENDIF

.IF "$(LIBPAPER_LINK)" == "YES"
SHL1STDLIBS += -lpaper
.ENDIF

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

.ENDIF      # "$(OS)"=="WNT"

.ENDIF # GUIBASE = aqua

# --- Targets ------------------------------------------------------------


.INCLUDE :	target.mk
