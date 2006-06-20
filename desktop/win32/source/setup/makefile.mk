#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: hr $ $Date: 2006-06-20 04:54:45 $
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

PRJNAME		=	desktop
TARGET		=	loader2
LIBTARGET	=	NO
DYNAMIC_CRT	=
TARGETTYPE	=	GUI

ENABLE_EXCEPTIONS	=	TRUE

# --- Settings ------------------------------------------------------------

.INCLUDE : settings.mk

# --- Allgemein -----------------------------------------------------------

RCFILES=	$(RES)$/$(TARGET).rc

ULFFILES=	setup.ulf

OBJFILES= 	$(OBJ)$/setup_main.obj \
            $(OBJ)$/setup_a.obj \
            $(OBJ)$/setup_w.obj

# --- Targets ------------------------------------------------------
# Generate the native Windows resource file
# using lngconvex.exe 

UWINAPILIB=		$(0)
LIBSALCPPRT=	$(0)

APP1NOSAL=		TRUE
APP1TARGET=		loader2

APP1STDLIBS=	gdi32.lib advapi32.lib libcmt.lib shell32.lib
APP1OBJS=		$(OBJFILES)

APP1DEF=		$(MISC)$/$(TARGET).def

APP1RES=		$(RES)$/$(TARGET).res
APP1NOSVRES=	$(RES)$/$(TARGET).res

# --- setup --------------------------------------------------------------

.INCLUDE :  target.mk

$(RCFILES) : $(COMMONMISC)$/$(TARGET)$/setup.ulf makefile.mk rcfooter.txt rcheader.txt rctmpl.txt
    $(WRAPCMD) lngconvex.exe -ulf $(COMMONMISC)$/$(TARGET)$/setup.ulf -rc $(RCFILES) -rct rctmpl.txt -rch rcheader.txt -rcf rcfooter.txt

