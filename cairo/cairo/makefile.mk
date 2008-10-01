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
# $Revision: 1.1.2.4 $
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
TARGET=so_cairo

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

CAIROVERSION=1.6.4

TARFILE_NAME=$(PRJNAME)-$(CAIROVERSION)
PATCH_FILE_NAME=..$/$(TARFILE_NAME).patch

cairo_CFLAGS=-I$(SOLARINC)
cairo_LDFLAGS=-L$(SOLARLIB)

# pixman is in this module
pixman_CFLAGS=-I$(SRC_ROOT)$/$(PRJNAME)$/$(INPATH)$/inc
pixman_LIBS=-L$(SRC_ROOT)$/$(PRJNAME)$/$(INPATH)$/lib -lpixman-1

cairo_CPPFLAGS=
.IF "$(SYSTEM_ZLIB)"!="YES"
cairo_CPPFLAGS+=-I$(SOLARINCDIR)$/external$/zlib
cairo_COMPRESS=z_compress
.ELSE
cairo_COMPRESS=compress
.ENDIF
cairo_CPPFLAGS+=$(INCLUDE)

.IF "$(OS)"=="WNT"
# --------- Windows -------------------------------------------------
.IF "$(COM)"=="GCC"
cairo_CPPFLAGS+=-nostdinc

CONFIGURE_DIR=
CONFIGURE_ACTION=cp $(SRC_ROOT)$/$(PRJNAME)$/cairo$/dummy_pkg_config . && .$/configure
CONFIGURE_FLAGS=--disable-xlib --disable-freetype --disable-pthread --disable-svg --disable-png --enable-gtk-doc=no --enable-test-surfaces=no --enable-static=no --build=i586-pc-mingw32 --host=i586-pc-mingw32 PKG_CONFIG=./dummy_pkg_config CFLAGS=-D_MT CPPFLAGS="$(cairo_CPPFLAGS)" LDFLAGS="$(cairo_LDFLAGS) -no-undefined -L$(ILIB:s/;/ -L/)" LIBS=-lmingwthrd pixman_CFLAGS="$(pixman_CFLAGS)" pixman_LIBS="$(pixman_LIBS)" ZLIB3RDLIB=$(ZLIB3RDLIB) COMPRESS=$(cairo_COMPRESS) OBJDUMP="$(WRAPCMD) objdump"
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

OUT2INC+=src$/cairo-win32.h

.ELIF "$(GUIBASE)"=="aqua"
# ----------- Native Mac OS X (Aqua/Quartz) --------------------------------
CONFIGURE_DIR=
CONFIGURE_ACTION=cp $(SRC_ROOT)$/$(PRJNAME)$/cairo$/dummy_pkg_config . && .$/configure
CONFIGURE_FLAGS=--enable-static=no --disable-xlib --disable-freetype --disable-svg --disable-png --enable-quartz --enable-quartz-font --enable-gtk-doc=no --enable-test-surfaces=no PKG_CONFIG=./dummy_pkg_config CFLAGS="$(cairo_CFLAGS)" CPPFLAGS="$(cairo_CPPFLAGS)" LDFLAGS="$(cairo_LDFLAGS)" pixman_CFLAGS="$(pixman_CFLAGS)" pixman_LIBS="$(pixman_LIBS)" ZLIB3RDLIB=$(ZLIB3RDLIB) COMPRESS=$(cairo_COMPRESS)
BUILD_ACTION=$(GNUMAKE)
BUILD_FLAGS+= -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)

OUT2INC+=src$/cairo-quartz.h

.ELSE
# ----------- Unix ---------------------------------------------------------
.IF "$(OS)$(COM)"=="LINUXGCC" || "$(OS)$(COM)"=="FREEBSDGCC"
LDFLAGS:=-Wl,-rpath,'$$$$ORIGIN:$$$$ORIGIN/../ure-link/lib' -Wl,-noinhibit-exec -Wl,-z,noexecstack
.ELIF "$(OS)$(COM)"=="SOLARISC52"
LDFLAGS:=-Wl,-R'$$$$ORIGIN:$$$$ORIGIN/../ure-link/lib'
.ELIF "$(OS)"=="MACOSX"      # X11 on Mac OS X
cairo_LDFLAGS+=-lfontconfig -lXrender
.ENDIF  # "$(OS)$(COM)"=="LINUXGCC" || "$(OS)$(COM)"=="FREEBSDGCC"

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
CONFIGURE_FLAGS=--enable-xlib --enable-freetype --disable-svg --disable-png --enable-gtk-doc=no --enable-test-surfaces=no --enable-static=no CFLAGS="$(cairo_CFLAGS)" CPPFLAGS="$(cairo_CPPFLAGS)" LDFLAGS="$(cairo_LDFLAGS)" pixman_CFLAGS="$(pixman_CFLAGS)" pixman_LIBS="$(pixman_LIBS)" ZLIB3RDLIB=$(ZLIB3RDLIB) COMPRESS=$(cairo_COMPRESS)
.IF "$(OS)"=="MACOSX"      # X11 on Mac OS X
CONFIGURE_ACTION=cp $(SRC_ROOT)$/$(PRJNAME)$/cairo$/dummy_pkg_config . && .$/configure
CONFIGURE_FLAGS+=--disable-quartz --disable-quartz-font PKG_CONFIG=./dummy_pkg_config
.ENDIF
BUILD_ACTION=$(GNUMAKE)
BUILD_FLAGS+= -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)

OUT2INC+=src$/cairo-xlib.h \
     src$/cairo-xlib-xrender.h

.ENDIF



# -------- All platforms --------------------------------------------



OUT2INC+=src$/cairo-deprecated.h \
     src$/cairo-features.h  \
     src$/cairo-pdf.h	\
     src$/cairo-ps.h	\
     src$/cairo.h

.IF "$(OS)"=="MACOSX"
OUT2LIB+=src$/.libs$/libcairo*.dylib
.ELIF "$(OS)"=="WNT"
.IF "$(COM)"=="GCC"
OUT2BIN+=src$/.libs$/*.a
OUT2BIN+=src$/.libs$/*.dll
.ELSE
OUT2LIB+=win32$/bin.msvc$/*.lib
OUT2BIN+=win32$/bin.msvc$/*.dll
.ENDIF
.ELSE
OUT2LIB+=src$/.libs$/libcairo.so*
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

