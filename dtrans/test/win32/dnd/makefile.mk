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
# $Revision: 1.16 $
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

PRJNAME=dtrans
TARGET=dndTest
TARGETTYPE=CUI
LIBTARGET=NO

#USE_DEFFILE=	TRUE
ENABLE_EXCEPTIONS=TRUE

# --- Settings ---

.INCLUDE : settings.mk

# --- Files ---

# CFLAGS+=-GR -DUNICODE -D_UNICODE
CFLAGS+= -D_WIN32_DCOM

INCPRE+=	-I$(ATL_INCLUDE)

OBJFILES=	$(OBJ)$/dndTest.obj	\
        $(OBJ)$/atlwindow.obj \
        $(OBJ)$/targetlistener.obj \
        $(OBJ)$/sourcelistener.obj \
        $(OBJ)$/dataobject.obj
        
APP1NOSAL=TRUE

APP1TARGET=	$(TARGET)
APP1OBJS=$(OBJFILES)

APP1STDLIBS= \
    $(SALLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB)		\
    $(UWINAPILIB) \
    $(USER32LIB)		\
    $(OLE32LIB)		\
    comsupp.lib		\
    $(OLEAUT32LIB)	\
    $(GDI32LIB)		\
    $(UUIDLIB)

APP1LIBS=	\
            $(SLB)$/dtobjfact.lib	\
            $(SLB)$/dtutils.lib

#			$(SOLARLIBDIR)$/imtaolecb.lib\

APP1DEF=	$(MISC)\$(APP1TARGET).def

# --- Targets ---

.INCLUDE : target.mk

