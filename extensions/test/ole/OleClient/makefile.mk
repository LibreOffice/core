#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.10 $
#
#   last change: $Author: vg $ $Date: 2006-09-25 12:45:58 $
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
    $(LIBCIMT) 	\
    user32.lib	\
    $(COMPATH)$/atlmfc$/lib$/atls.lib \
    kernel32.lib \
    ole32.lib	\
    oleaut32.lib	\
    uuid.lib		\
    gdi32.lib	\
    advapi32.lib

APP1DEF=	$(MISC)\$(APP1TARGET).def

# --- Targets ---

.INCLUDE : target.mk


