#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.15 $
#
#   last change: $Author: vg $ $Date: 2007-03-26 15:09:28 $
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

