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
TARGET=so_cairo
EXTERNAL_WARNINGS_NOT_ERRORS := TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(SYSTEM_CAIRO)" == "YES"
all:
    @echo "Not building cairo."
.ENDIF

# --- Files --------------------------------------------------------

CAIROVERSION=1.10.2

TARFILE_NAME=$(PRJNAME)-$(CAIROVERSION)
TARFILE_MD5=f101a9e88b783337b20b2e26dfd26d5f

PATCH_FILES=\
    ..$/cairo-1.10.2.patch \
    ..$/cairo.dlsym.lcdfilter.patch \
    ..$/cairo-1.10.2-oldfontconfig.patch

.IF "$(OS)$(COM)" == "WNTMSC"
PATCH_FILES+= ..$/$(TARFILE_NAME).wntmsc.patch
.ENDIF

.IF "$(OS)" == "IOS"
PATCH_FILES+=..$/$(TARFILE_NAME).no-atsui.patch
PATCH_FILES+=..$/$(TARFILE_NAME).ios.patch
.ENDIF

.IF "$(OS)" == "ANDROID"
PATCH_FILES+=..$/$(TARFILE_NAME).android.patch
.ENDIF

cairo_CFLAGS=$(SOLARINC)
cairo_LDFLAGS=-L$(SOLARVER)$/$(INPATH)$/lib

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
cairo_LDFLAGS+=-no-undefined -L$(ILIB:s/;/ -L/)
cairo_CPPFLAGS+=-nostdinc
cairo_CC=$(CC) -mthreads

.IF "$(MINGW_SHARED_GCCLIB)"=="YES"
cairo_CC+=-shared-libgcc
.ENDIF
.IF "$(MINGW_SHARED_GXXLIB)"=="YES"
cairo_LIBS+=$(MINGW_SHARED_LIBSTDCPP)
.ENDIF

CONFIGURE_DIR=
CONFIGURE_ACTION=cp $(SRC_ROOT)$/$(PRJNAME)$/cairo$/dummy_pkg_config . && .$/configure
CONFIGURE_FLAGS=--disable-valgrind --disable-xlib --disable-ft --disable-pthread --disable-svg --enable-gtk-doc=no --enable-test-surfaces=no --enable-static=no --build=i586-pc-mingw32 --host=i586-pc-mingw32 PKG_CONFIG=./dummy_pkg_config CC="$(cairo_CC)" LIBS="$(cairo_LIBS)" ZLIB3RDLIB=$(ZLIB3RDLIB) COMPRESS=$(cairo_COMPRESS) OBJDUMP="$(WRAPCMD) objdump"
BUILD_ACTION=$(GNUMAKE)
BUILD_FLAGS+= -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)$/src
.IF "$(GUI)$(COM)"=="WNTGCC"
.EXPORT : PWD
.ENDIF

.ELSE   # WNT, not GCC
BUILD_ACTION=$(GNUMAKE) -f Makefile.win32 CFG=release ZLIB3RDLIB=$(ZLIB3RDLIB)
BUILD_DIR=
.ENDIF

OUT2INC+=src$/cairo-win32.h

.ELIF "$(GUIBASE)"=="aqua"
# ----------- Native Mac OS X (Aqua/Quartz) --------------------------------
.IF "$(SYSBASE)"!=""
.IF "$(EXTRA_CFLAGS)" != ""
cairo_CFLAGS+=$(EXTRA_CFLAGS) $(EXTRA_CDEFS)
cairo_CPPFLAGS+=$(EXTRA_CFLAGS) $(EXTRA_CDEFS)
.ENDIF # "$(EXTRA_CFLAGS)" != ""
.ENDIF # "$(SYSBASE)"!=""
CONFIGURE_DIR=
CONFIGURE_ACTION=cp $(SRC_ROOT)$/$(PRJNAME)$/cairo$/dummy_pkg_config . && .$/configure
.IF $(MACOSX_SDK_VERSION) >= 1070 
PATCH_FILES+=..$/$(TARFILE_NAME).no-atsui.patch
.ENDIF
CONFIGURE_FLAGS=--enable-static=no --disable-valgrind --disable-xlib --disable-ft --disable-svg --enable-quartz --enable-quartz-font --enable-gtk-doc=no --enable-test-surfaces=no PKG_CONFIG=./dummy_pkg_config ZLIB3RDLIB=$(ZLIB3RDLIB) COMPRESS=$(cairo_COMPRESS)
.IF "$(CROSS_COMPILING)"=="YES"
CONFIGURE_FLAGS+=--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)
.ENDIF
cairo_CPPFLAGS+=$(EXTRA_CDEFS)
cairo_LDFLAGS+=$(EXTRA_LINKFLAGS)
BUILD_ACTION=$(GNUMAKE)
BUILD_FLAGS+= -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)$/src

OUT2INC+=src$/cairo-quartz.h

.ELSE
# ----------- Unix ---------------------------------------------------------
.IF "$(OS)$(COM)"=="LINUXGCC" || "$(OS)$(COM)"=="FREEBSDGCC"
cairo_LDFLAGS+=-Wl,-z,origin -Wl,-rpath,\$$$$ORIGIN:\$$$$ORIGIN/../ure-link/lib -Wl,-z,noexecstack
.ELIF "$(OS)$(COM)"=="SOLARISC52"
cairo_LDFLAGS+=-Wl,-R\$$$$ORIGIN:\$$$$ORIGIN/../ure-link/lib
.ENDIF  # "$(OS)$(COM)"=="LINUXGCC" || "$(OS)$(COM)"=="FREEBSDGCC"

.IF "$(SYSBASE)"!=""
cairo_CFLAGS+=-I$(SYSBASE)$/usr$/include -I$(SOLARINCDIR)$/external $(EXTRA_CFLAGS)
.IF "$(OS)"=="SOLARIS" || "$(OS)"=="LINUX"
cairo_LDFLAGS+=-L$(SYSBASE)$/lib -L$(SYSBASE)$/usr$/lib -L$(SOLARLIBDIR) -lpthread -ldl
.ENDIF
.ENDIF			# "$(SYSBASE)"!=""

LDFLAGS:=$(cairo_LDFLAGS)
.EXPORT: LDFLAGS

.IF "$(COMNAME)"=="sunpro5"
cairo_CFLAGS+=-xc99=none
.ENDIF

.IF "$(CPUNAME)"=="INTEL"
cairo_CFLAGS+=-march=i486
.ENDIF

CONFIGURE_DIR=

.IF "$(OS)"=="IOS" || "$(OS)"=="ANDROID"
# No pkg-config on MacOSX (for iOS) or in the Android NDK
CONFIGURE_ACTION=cp $(SRC_ROOT)$/$(PRJNAME)$/cairo$/dummy_pkg_config . && .$/configure
.ELSE
CONFIGURE_ACTION=.$/configure
.ENDIF

.IF "$(OS)"=="IOS"
CONFIGURE_FLAGS=--disable-shared
.ELSE
CONFIGURE_FLAGS=--disable-static
.ENDIF

.IF "$(OS)"=="IOS" || "$(OS)"=="ANDROID"
CONFIGURE_FLAGS+=--disable-xlib
.ELSE
CONFIGURE_FLAGS+=--enable-xlib
.ENDIF

.IF "$(OS)"=="IOS"
CONFIGURE_FLAGS+=--enable-quartz --enable-quartz-font
.ENDIF

CONFIGURE_FLAGS+=--disable-valgrind

.IF "$(OS)"=="IOS"
CONFIGURE_FLAGS+=--disable-ft
.ELSE
CONFIGURE_FLAGS+=--enable-ft --enable-fc
.ENDIF

.IF "$(OS)"=="IOS" || "$(OS)"=="ANDROID"
CONFIGURE_FLAGS+=PKG_CONFIG=./dummy_pkg_config
.ENDIF

CONFIGURE_FLAGS+=--disable-svg --enable-gtk-doc=no --enable-test-surfaces=no ZLIB3RDLIB=$(ZLIB3RDLIB) COMPRESS=$(cairo_COMPRESS)

.IF "$(CROSS_COMPILING)"=="YES"
CONFIGURE_FLAGS+=--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)
.ENDIF

BUILD_ACTION=$(GNUMAKE)
BUILD_FLAGS+= -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)$/src

.IF "$(OS)" == "IOS"
OUT2INC+=src$/cairo-quartz.h
.ELSE
OUT2INC+=src$/cairo-xlib.h \
     src$/cairo-xlib-xrender.h \
     src$/cairo-ft.h
.ENDIF

.ENDIF


# -------- All platforms --------------------------------------------

.IF "$(OS)" != "WNT" || "$(COM)" == "GCC"
# all other platforms except vanilla WNT, which does not use configure

# pixman is in this module
# We include paths to this module also in LDFLAGS/CFLAGS to guarantee search order.
# However pixman_* vars need to be also set for configure to work properly on all platforms.
CONFIGURE_FLAGS+=pixman_CFLAGS="-I$(SRC_ROOT)$/$(PRJNAME)$/$(INPATH)$/inc" pixman_LIBS="-L$(SRC_ROOT)$/$(PRJNAME)$/$(INPATH)$/lib -lpixman-1"

.IF "$(debug)"!=""
cairo_CFLAGS+=-g
CONFIGURE_FLAGS+= STRIP=" "
.ENDIF

CONFIGURE_FLAGS+=CFLAGS="-I$(SRC_ROOT)$/$(PRJNAME)$/$(INPATH)$/inc $(cairo_CFLAGS)" LDFLAGS='-L$(SRC_ROOT)$/$(PRJNAME)$/$(INPATH)$/lib $(cairo_LDFLAGS)' CPPFLAGS="$(cairo_CPPFLAGS)"

.ENDIF

.IF "$(SYSTEM_LIBPNG)" != "YES"
CONFIGURE_FLAGS+=png_CFLAGS="-I$(SOLARINCDIR)$/external$/libpng" png_LIBS="-L$(SOLARLIBDIR) -lpng"
.ENDIF

.IF "$(OS)" == "MACOSX"
CONFIGURE_FLAGS += \
    --prefix=/@.__________________________________________________$(EXTRPATH)
.END

OUT2INC+=cairo-version.h \
     src$/cairo-deprecated.h \
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
OUT2LIB+=src$/release$/*.lib
OUT2BIN+=src$/release$/*.dll
.ENDIF
.ELIF "$(OS)"=="IOS"
OUT2LIB+=src$/.libs$/libcairo*.a
.ELSE
OUT2LIB+=src$/.libs$/libcairo.so*
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

