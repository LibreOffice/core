#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: hjs $ $Date: 2003-08-18 15:12:27 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
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
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
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
TARGET2=crash_report_with_gtk.bin
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

.IF "$(ENABLE_CRASHDUMP)" != "" || "$(PRODUCT)" != "FULL"

CFLAGS+=`pkg-config --cflags gtk+-2.0`

SOLARLIB!:=$(SOLARLIB:s/jre/jnore/)

OBJFILES=\
    $(OBJ)$/interface.obj            \
    $(OBJ)$/res.obj                  \
    $(OBJ)$/main.obj

APP1NOSAL=TRUE
APP1TARGET=$(TARGET)
APP1OBJS=$(OBJFILES)

APP1STDLIBS=`pkg-config --libs gtk+-2.0` $(DYNAMIC) -lXext -lX11 -ldl -lnsl
.IF "$(OS)" == "SOLARIS"
APP1STDLIBS+=-lsocket
.ENDIF
.IF "$(OS)" == "FREEBSD"
APP1STDLIBS=`pkg-config --libs gtk+-2.0` $(DYNAMIC) -lXext -lX11
.ENDIF



# Build statically linked version for special builds and non-pros

.IF "$(ENABLE_CRASHDUMP)" == "STATIC" || "$(PRODUCT)" != "FULL"
APP2NOSAL=TRUE
APP2TARGET=$(TARGET2)
APP2OBJS=$(OBJFILES)

APP2STDLIBS=$(STATIC) `pkg-config --only-mod-libs --libs gtk+-2.0` -lpng -lzlib -ljpeg -ltiff $(DYNAMIC) -lXext -lX11 -ldl -lnsl
.IF "$(OS)" == "SOLARIS"
APP2STDLIBS+=-lsocket
.ENDIF
.IF "$(OS)" == "FREEBSD"
APP2STDLIBS=$(STATIC) `pkg-config --libs gtk+-2.0` -lpng -lzlib -ljpeg -ltiff $(DYNAMIC) -lXext -lX11
.ENDIF

.ENDIF
# Building crash_report_static

ALL: ALLTAR $(BIN)$/crash_dump.res.01

.ENDIF #  "$(ENABLE_CRASHDUMP)" != "" || "$(PRODUCT)" != "FULL"

.ENDIF #  "$(OS)"=="MACOSX"

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

$(OBJ)$/main.obj: $(INCCOM)$/_version.h

$(BIN)$/crash_dump.res.01: ..$/all$/crashrep.lng
    $(BIN)$/unxcrashres ..$/all$/crashrep.lng $(BIN)$/crash_dump.res

# Building crash_report
