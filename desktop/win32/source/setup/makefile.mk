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

PRJ=..$/..$/..

PRJNAME		=	desktop
TARGET		=	loader2
LIBTARGET	=	NO
DYNAMIC_CRT	=
TARGETTYPE	=	GUI

ENABLE_EXCEPTIONS	=	TRUE

# --- Settings ------------------------------------------------------------

.INCLUDE : settings.mk

# --- Allgemein -----------------------------------------------------------

INCPRE+=$(MFC_INCLUDE)

.IF "$(USE_DEBUG_RUNTIME)"!=""
CDEFS+=-D_DEBUG
.ENDIF # "$(USE_DEBUG_RUNTIME)"!=""

RCFILES=	$(RES)$/$(TARGET).rc

ULFFILES=	setup.ulf

.IF "$(WITH_LANG)"!=""
ULFDIR:=$(COMMONMISC)$/$(TARGET)
.ELSE			# "$(WITH_LANG)"!=""
ULFDIR:=.
.ENDIF			# "$(WITH_LANG)"!=""

OBJFILES= 	$(OBJ)$/setup_main.obj \
            $(OBJ)$/setup_w.obj

# --- Targets ------------------------------------------------------
# Generate the native Windows resource file
# using lngconvex.exe 

UWINAPILIB=		$(0)
LIBSALCPPRT=	$(0)

APP1NOSAL=		TRUE
APP1TARGET=		loader2

APP1STDLIBS=	$(GDI32LIB) $(ADVAPI32LIB) $(SHELL32LIB) $(MSILIB)
.IF "$(COM)"!="GCC"
APP1STDLIBS+=	libcmt.lib
.ENDIF
APP1OBJS=		$(OBJFILES)

APP1DEF=		$(MISC)$/$(TARGET).def

APP1RES=		$(RES)$/$(TARGET).res
APP1NOSVRES=	$(RES)$/$(TARGET).res

# --- setup --------------------------------------------------------------

.INCLUDE :  target.mk

$(RCFILES) : $(ULFDIR)$/setup.ulf makefile.mk rcfooter.txt rcheader.txt rctmpl.txt
    $(LNGCONVEX) -ulf $(ULFDIR)$/setup.ulf -rc $(RCFILES) -rct rctmpl.txt -rch rcheader.txt -rcf rcfooter.txt

