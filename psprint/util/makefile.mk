#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.23 $
#
#   last change: $Author: kz $ $Date: 2008-04-03 16:47:36 $
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

# --- Windows, OS/2 and Mac OS X Aqua Targets ----------------------------

.IF "$(OS)"=="WNT" || "$(OS)"=="OS2" || "$(GUIBASE)"=="aqua"
LIB1ARCHIV=$(LB)$/lib$(TARGET).a
LIB1TARGET= $(SLB)$/a$(TARGET).lib
LIB1FILES=	$(SLB)$/fontsubset.lib

# --- Other Targets ---
.ELSE

LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1FILES=	$(SLB)$/fontman.lib			\
            $(SLB)$/helper.lib			\
            $(SLB)$/printer.lib			\
            $(SLB)$/fontsubset.lib 		\
            $(SLB)$/gfx.lib

SHL1TARGET= psp$(DLLPOSTFIX)
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
            $(COMPHELPERLIB)    \
            $(CPPULIB)          \
            $(SALLIB)			\
            -lX11

.IF "$(LIBPAPER_LINK)" == "YES"
SHL1STDLIBS += -lpaper
.ENDIF

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

.ENDIF      # "$(OS)"=="WNT"

# --- Targets ------------------------------------------------------------


.INCLUDE :	target.mk
