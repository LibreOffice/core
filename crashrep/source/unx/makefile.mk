#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.13 $
#
#   last change: $Author: hr $ $Date: 2004-12-20 09:55:18 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
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
PKGCONFIG_MODULES=gtk+-2.0
.INCLUDE: pkg_config.mk

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

APP1STDLIBS=$(DYNAMIC) -lXext -lX11 -ldl -lnsl
.IF "$(OS)" == "SOLARIS"
APP1STDLIBS+=-lsocket
.ENDIF

.ENDIF #  "$(ENABLE_CRASHDUMP)" != "" || "$(PRODUCT)" == ""

.ENDIF #  "$(OS)"=="MACOSX"

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

$(OBJ)$/main.obj: $(INCCOM)$/_version.h

# Building crash_report
