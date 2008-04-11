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
# $Revision: 1.2 $
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

PRJNAME=xpdf
TARGET=xpdfimport
TARGETTYPE=CUI
ENABLE_EXCEPTIONS=TRUE
EXTERNAL_WARNINGS_NOT_ERRORS := TRUE

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

# --- Files --------------------------------------------------------

APP1TARGET=$(TARGET)
APP1LIBSALCPPRT=
APP1OBJS= \
    $(OBJ)$/wrapper_gpl.obj $(OBJ)/pdfioutdev_gpl.obj
.IF "$(GUI)" == "WNT"
APP1STDLIBS+=xpdf.lib fofi.lib Goo.lib gdi32.lib advapi32.lib
.ELSE
APP1STDLIBS+=-lxpdf -lfofi -lGoo
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
