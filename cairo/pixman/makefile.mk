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
# $Revision: 1.1.2.2 $
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

PRJNAME=cairo
TARGET=so_pixman

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF  "$(ENABLE_CAIRO)" == ""
all:
        @echo "Nothing to do (Cairo not enabled)."

.ELIF "$(SYSTEM_CAIRO)" == "YES"
all:
    @echo "Nothing to do, using system cairo."

.ELIF "$(BUILD_CAIRO)" == ""
all:
       @echo "Not building cairo from source, prebuilt binaries will be used."

.ENDIF

# --- Files --------------------------------------------------------

PIXMANVERSION=0.10.0

TARFILE_NAME=pixman-$(PIXMANVERSION)

# Note: we are building static pixman library to avoid linking problems.

.IF "$(OS)"=="WNT"
# --------- Windows -------------------------------------------------
.IF "$(COM)"=="GCC"
CONFIGURE_DIR=
CONFIGURE_ACTION=.$/configure
CONFIGURE_FLAGS=--enable-static=yes --enable-shared=no --build=i586-pc-mingw32 --host=i586-pc-mingw32 CFLAGS="$(cairo_CFLAGS) -D_MT" LDFLAGS="$(cairo_LDFLAGS) -no-undefined -L$(ILIB:s/;/ -L/)" LIBS="-lmingwthrd" OBJDUMP="$(WRAPCMD) objdump"
BUILD_ACTION=$(GNUMAKE)
BUILD_FLAGS+= -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)
.IF "$(GUI)$(COM)"=="WNTGCC"
.EXPORT : PWD
.ENDIF

.ELSE   # WNT, not GCC
CONFIGURE_DIR=win32
CONFIGURE_ACTION=cscript configure.js
.IF "$(debug)"!=""
CONFIGURE_FLAGS+=debug=yes
.ENDIF
BUILD_ACTION=nmake
BUILD_DIR=$(CONFIGURE_DIR)
.ENDIF

.ELIF "$(GUIBASE)"=="aqua"
# ----------- Native Mac OS X (Aqua/Quartz) --------------------------------
CONFIGURE_DIR=
CONFIGURE_ACTION=.$/configure
CONFIGURE_FLAGS=--enable-static=yes --enable-shared=no
BUILD_ACTION=$(GNUMAKE)
BUILD_FLAGS+= -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)

.ELSE
# ----------- Unix ---------------------------------------------------------
.IF "$(OS)$(COM)"=="LINUXGCC" || "$(OS)$(COM)"=="FREEBSDGCC"
LDFLAGS:=-Wl,-rpath,'$$$$ORIGIN:$$$$ORIGIN/../ure-link/lib' -Wl,-noinhibit-exec -Wl,-z,noexecstack
.ENDIF                  # "$(OS)$(COM)"=="LINUXGCC"

.IF "$(OS)$(COM)"=="SOLARISC52"
LDFLAGS:=-Wl,-R'$$$$ORIGIN:$$$$ORIGIN/../ure-link/lib'
.ENDIF                  # "$(OS)$(COM)"=="SOLARISC52"

.IF "$(SYSBASE)"!=""
cairo_CFLAGS+=-I$(SYSBASE)$/usr$/include -I$(SOLARINCDIR)$/external $(EXTRA_CFLAGS)
.IF "$(OS)"=="SOLARIS" || "$(OS)"=="LINUX"
LDFLAGS+=-L$(SYSBASE)$/lib -L$(SYSBASE)$/usr$/lib -L$(SOLARLIBDIR) -lpthread -ldl
.ENDIF
.ENDIF			# "$(SYSBASE)"!=""

.EXPORT: LDFLAGS

.IF "$(COMNAME)"=="sunpro5"
cairo_CFLAGS+=-xc99=none
.ENDIF

CONFIGURE_DIR=
CONFIGURE_ACTION=.$/configure
CONFIGURE_FLAGS=--enable-static=yes --enable-shared=no CFLAGS="$(cairo_CFLAGS)"
BUILD_ACTION=$(GNUMAKE)
BUILD_FLAGS+= -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)
.ENDIF



# -------- All platforms --------------------------------------------

OUT2INC=pixman$/pixman-version.h  \
    pixman$/pixman.h

.IF "$(OS)"=="MACOSX"
OUT2LIB+=pixman$/.libs$/libpixman-1.a
.ELIF "$(OS)"=="WNT"
.IF "$(COM)"=="GCC"
OUT2LIB+=pixman$/.libs$/*.a
.ELSE
OUT2LIB+=win32$/bin.msvc$/*.lib
OUT2BIN+=win32$/bin.msvc$/*.dll
.ENDIF
.ELSE
OUT2LIB+=pixman$/.libs$/libpixman-1.a
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

