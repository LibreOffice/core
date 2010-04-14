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

PRJ=..$/..$/..$/

PRJNAME=extensions
TARGET=clientTest
TARGETTYPE=CUI
LIBTARGET=NO

ENABLE_EXCEPTIONS=TRUE

# --- Settings ---

.INCLUDE : settings.mk

# --- Files ---

INCPRE+=	$(OUT)$/inc -I$(ATL_INCLUDE)



APP1TARGET=	$(TARGET)
APP1OBJS=  \
    $(OBJ)$/clientTest.obj \
    $(OBJ)$/axhost.obj \
    $(OBJ)$/funcs.obj

APP1STDLIBS= \
    $(SALLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(USER32LIB)	\
    $(COMPATH)$/atlmfc$/lib$/atls.lib \
    $(KERNEL32LIB) \
    $(OLE32LIB)	\
    $(OLEAUT32LIB)	\
    $(UUIDLIB)		\
    $(GDI32LIB)	\
    $(ADVAPI32LIB)    

APP1DEF=	$(MISC)\$(APP1TARGET).def

# --- Targets ---

.INCLUDE : target.mk


