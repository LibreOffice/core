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

PRJ=..

PRJNAME=cairo
TARGET=so_pixman
EXTERNAL_WARNINGS_NOT_ERRORS := TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF  "$(ENABLE_CAIRO)" == ""
all:
    @echo "Nothing to do (Cairo not enabled)."

.ELIF "$(BUILD_PIXMAN)" == ""
all:
    @echo "Not building pixman."

.ENDIF

# --- Files --------------------------------------------------------

PIXMANVERSION=0.12.0

TARFILE_NAME=pixman-$(PIXMANVERSION)
TARFILE_MD5=09357cc74975b01714e00c5899ea1881

PATCH_FILES=..$/$(TARFILE_NAME).patch

# Note: we are building static pixman library to avoid linking problems.
# However, for Unix dynamic library must be used (especially due to 64bit issues)

.IF "$(OS)"=="WNT"
# --------- Windows -------------------------------------------------
.IF "$(COM)"=="GCC"
CONFIGURE_DIR=
CONFIGURE_ACTION=.$/configure
CONFIGURE_FLAGS=--enable-static=yes --enable-shared=no --build=i586-pc-mingw32 --host=i586-pc-mingw32 CFLAGS="$(pixman_CFLAGS) -mthreads" LDFLAGS="$(pixman_LDFLAGS) -no-undefined -L$(ILIB:s/;/ -L/)" OBJDUMP="$(WRAPCMD) objdump"
BUILD_ACTION=$(GNUMAKE)
BUILD_FLAGS+= -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)
.IF "$(GUI)$(COM)"=="WNTGCC"
.EXPORT : PWD
.ENDIF

.ELSE   # WNT, not GCC
BUILD_DIR=pixman
BUILD_ACTION=$(GNUMAKE) -f Makefile.win32
.ENDIF

.ELIF "$(GUIBASE)"=="aqua"
# ----------- Native Mac OS X (Aqua/Quartz) --------------------------------
CONFIGURE_DIR=
CONFIGURE_ACTION=.$/configure
CONFIGURE_FLAGS=--enable-static=yes --enable-shared=no CPPFLAGS="$(EXTRA_CDEFS)"
.IF "$(SYSBASE)"!=""
.IF "$(EXTRA_CFLAGS)"!=""
CONFIGURE_FLAGS+=CFLAGS="$(EXTRA_CFLAGS) $(EXTRA_CDEFS)"
.ENDIF # "$(EXTRA_CDEFS)"!=""
.ENDIF # "$(SYSBASE)"!=""
BUILD_ACTION=$(GNUMAKE)
BUILD_FLAGS+= -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)

.ELSE
# ----------- Unix ---------------------------------------------------------
.IF "$(OS)$(COM)"=="LINUXGCC" || "$(OS)$(COM)"=="FREEBSDGCC"
LDFLAGS:=-Wl,-rpath,'$$$$ORIGIN:$$$$ORIGIN/../ure-link/lib' -Wl,-noinhibit-exec
.ENDIF                  # "$(OS)$(COM)"=="LINUXGCC"

.IF "$(OS)$(COM)"=="SOLARISC52"
LDFLAGS:=-Wl,-R'$$$$ORIGIN:$$$$ORIGIN/../ure-link/lib'
.ENDIF                  # "$(OS)$(COM)"=="SOLARISC52"

.IF "$(SYSBASE)"!=""
pixman_CFLAGS+=-I$(SYSBASE)$/usr$/include -I$(SOLARINCDIR)$/external $(EXTRA_CFLAGS)
.IF "$(OS)"=="SOLARIS" || "$(OS)"=="LINUX"
LDFLAGS+=-L$(SYSBASE)$/lib -L$(SYSBASE)$/usr$/lib -L$(SOLARLIBDIR) -lpthread -ldl
.ENDIF
.ENDIF			# "$(SYSBASE)"!=""

.EXPORT: LDFLAGS

.IF "$(COMNAME)"=="sunpro5"
pixman_CFLAGS+=-xc99=none
.ENDIF

.IF "$(CPU)"=="I"
pixman_CFLAGS+=-march=i486
.ENDIF

pixman_CFLAGS+=-fPIC

CONFIGURE_DIR=
CONFIGURE_ACTION=.$/configure
.IF "$(CPUNAME)"=="X86_64"
# static builds tend to fail on 64bit
CONFIGURE_FLAGS=--enable-static=no --enable-shared=yes
.ELSE
# use static lib to avoid linking problems with older system pixman libs
CONFIGURE_FLAGS=--enable-static=yes --enable-shared=no
.ENDIF
CONFIGURE_FLAGS+=CFLAGS="$(pixman_CFLAGS)"
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
OUT2LIB+=pixman$/release$/*.lib
.ENDIF
.ELSE
.IF "$(CPUNAME)"=="X86_64"
OUT2LIB+=pixman$/.libs$/libpixman-1.so
.ELSE
OUT2LIB+=pixman$/.libs$/libpixman-1.a
.ENDIF
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

