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
# $Revision: 1.24 $
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
