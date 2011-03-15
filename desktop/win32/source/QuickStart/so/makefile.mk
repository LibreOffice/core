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

PRJ=..$/..$/..$/..

PRJNAME=sysui
TARGET=soquickstart
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
TARGETTYPE=GUI

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

UWINAPILIB =

# --- Resources ----------------------------------------------------

.IF "$(LINK_SO)"=="TRUE"

RCFILES=QuickStart.rc
INCPRE=..

# --- Files --------------------------------------------------------

OBJFILES=$(OBJ)$/QuickStart.obj

APP1OBJS=$(OBJFILES)
APP1NOSAL=TRUE
APP1TARGET=$(TARGET)
APP1RPATH=BRAND
.IF "$(COM)"=="GCC"
APP1STDLIBS=-luuid
.ELSE
APP1STDLIBS=comsupp.lib
.ENDIF

APP1STDLIBS+=$(SHELL32LIB)\
            $(OLE32LIB)\
            $(GDI32LIB)\
            $(OLEAUT32LIB)\
            $(COMDLG32LIB)\
            $(KERNEL32LIB)\
            $(OLEAUT32LIB)

APP1NOSVRES=$(RES)$/$(TARGET).res

.ENDIF # "$(LINK_SO)"=="TRUE"

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
