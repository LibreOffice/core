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
# $Revision: 1.18 $
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

PRJ=..$/..

PRJNAME=crashrep
TARGET=crash_report.bin
TARGETTYPE=CUI

ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO
LIBSALCPPRT=$(0)

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# ------------------------------------------------------------------

.IF "$(OS)"=="MACOSX"

dummy:
    @echo "Nothing to build for OS $(OS)"

.ELSE		# "$(OS)"=="MACOSX"

# Only build crash reporter if either a product build with debug info
# or a non-pro build is done.

.IF "$(ENABLE_CRASHDUMP)" != "" || "$(PRODUCT)" == ""

SOLARLIB!:=$(SOLARLIB:s/jre/jnore/)

OBJFILES=\
    $(OBJ)$/main.obj

APP1NOSAL=TRUE
APP1TARGET=$(TARGET)
APP1OBJS=$(OBJFILES)
APP1RPATH=BRAND

APP1STDLIBS=$(DYNAMIC) -lXext -lX11
.IF "$(OS)" != "FREEBSD"
APP1STDLIBS+=-ldl -lnsl
.ENDIF
.IF "$(OS)" == "SOLARIS"
APP1STDLIBS+=-lsocket
.ENDIF

.ENDIF #  "$(ENABLE_CRASHDUMP)" != "" || "$(PRODUCT)" == ""

.ENDIF #  "$(OS)"=="MACOSX"

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

$(OBJ)$/main.obj: $(INCCOM)$/_version.h

# Building crash_report
