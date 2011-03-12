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

PRJNAME=desktop
TARGET=rebasegui
LIBTARGET=NO
TARGETTYPE=GUI
UWINAPILIB=

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

OBJFILES = \
    $(OBJ)$/rebase.obj	\
    $(OBJ)$/rebasegui.obj

ULFFILES=	rebasegui.ulf

.IF "$(WITH_LANG)"!=""
ULFDIR:=$(COMMONMISC)$/$(TARGET)
.ELSE			# "$(WITH_LANG)"!=""
ULFDIR:=.
.ENDIF			# "$(WITH_LANG)"!=""

RCFILES=		$(RES)$/$(TARGET).rc

# --- Targets ------------------------------------------------------

APP1NOSAL=		TRUE
APP1TARGET=		rebaseoo

APP1STDLIBS=	$(SHELL32LIB) $(SOLARLIBDIR)$/pathutils-obj.obj
.IF "$(COM)"=="GCC"
    APP1STDLIBS+=$(PSDK_HOME)$/lib$/imagehlp.lib
.ELSE
    APP1STDLIBS+=imagehlp.lib
.ENDIF

APP1OBJS=		$(OBJ)$/rebase.obj

APP1RPATH=		BRAND

APP2NOSAL=		TRUE
APP2TARGET=		rebasegui 
APP2STDLIBS=	$(SHELL32LIB) $(SOLARLIBDIR)$/pathutils-obj.obj

APP2OBJS=		$(OBJ)$/rebasegui.obj

APP2DEF=		$(MISC)$/$(TARGET).def
APP2RPATH=		BRAND

APP2RES=		$(RES)$/$(TARGET).res
APP2NOSVRES=	$(RES)$/$(TARGET).res

# --- setup --------------------------------------------------------------

.INCLUDE :  target.mk

$(RCFILES) : $(ULFDIR)$/rebasegui.ulf makefile.mk rcfooter.txt rcheader.txt rctmpl.txt
    $(LNGCONVEX) -ulf $(ULFDIR)$/rebasegui.ulf -rc $(RCFILES) -rct rctmpl.txt -rch rcheader.txt -rcf rcfooter.txt
