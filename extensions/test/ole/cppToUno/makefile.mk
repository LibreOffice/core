#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: vg $ $Date: 2006-09-25 12:46:27 $
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

PRJ=..$/..$/..

PRJNAME=extensions
TARGET=testcppuno
TARGETTYPE=CUI
LIBTARGET=NO

ENABLE_EXCEPTIONS=TRUE

# --- Settings ---

.INCLUDE : settings.mk

# --- Files ---

INCPRE+=-I$(ATL_INCLUDE)



APP1TARGET=	$(TARGET)
APP1OBJS=	$(OBJ)$/testcppuno.obj

LIBCMT=msvcrtd.lib


APP1STDLIBS= \
    $(SALLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    user32.lib	\
    kernel32.lib \
    ole32.lib	\
    oleaut32.lib	\
    uuid.lib		\
    comdlg32.lib	\
    gdi32.lib

.IF "$(GUI)"=="WNT"
APP1STDLIBS += $(LIBCIMT)
.ENDIF

APP1DEF=	$(MISC)$/$(APP1TARGET).def

# --- Targets ---
.INCLUDE : target.mk
