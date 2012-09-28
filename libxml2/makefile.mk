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

PRJ=.

PRJNAME=libxml2
TARGET=so_libxml2

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(SYSTEM_LIBXML)" == "YES"
all:
	@echo "An already available installation of libxml should exist on your system."
	@echo "Therefore the version provided here does not need to be built in addition."
.ENDIF

# --- Files --------------------------------------------------------

LIBXML2VERSION=2.7.6

TARFILE_NAME=$(PRJNAME)-$(LIBXML2VERSION)
TARFILE_MD5=7740a8ec23878a2f50120e1faa2730f2

# libxml2-global-symbols: #i112480#: Solaris ld won't export non-listed symbols
PATCH_FILES=libxml2-configure.patch \
            libxml2-gnome599717.patch \
            libxml2-xpath.patch \
            libxml2-global-symbols.patch \
            libxml2-aix.patch \
            libxml2-vc10.patch \
            libxml2-latin.patch

.IF "$(OS)" == "WNT"
PATCH_FILES+= libxml2-long-path.patch
.IF "$(COM)"=="GCC"
PATCH_FILES+= libxml2-mingw.patch
.ENDIF
.ENDIF

.IF "$(OS)"=="ANDROID"
PATCH_FILES+= libxml2-android.patch
.ENDIF

# This is only for UNX environment now

.IF "$(OS)"=="WNT"
.IF "$(COM)"=="GCC"
xml2_CC=$(CC) -mthreads
.IF "$(MINGW_SHARED_GCCLIB)"=="YES"
xml2_CC+=-shared-libgcc
.ENDIF
xml2_LIBS=-lws2_32
.IF "$(MINGW_SHARED_GXXLIB)"=="YES"
xml2_LIBS+=$(MINGW_SHARED_LIBSTDCPP)
.ENDIF
CONFIGURE_DIR=
CONFIGURE_ACTION=.$/configure
.IF "$(CROSS_COMPILING)"=="YES"
BUILD_AND_HOST=--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)
.ELSE
BUILD_AND_HOST=--build=i586-pc-mingw32 --host=i586-pc-mingw32
.ENDIF
.IF "$(ILIB)" == ""
CONF_ILIB=
.ELSE
CONF_ILIB=-L$(ILIB:s/;/ -L/)
.ENDIF
CONFIGURE_FLAGS=--enable-ipv6=no --without-python --without-zlib --enable-static=no --without-debug $(BUILD_AND_HOST) lt_cv_cc_dll_switch="-shared" CC="$(xml2_CC)" LDFLAGS="-Wl,--no-undefined -Wl,--enable-runtime-pseudo-reloc-v2 $(CONF_ILIB)" LIBS="$(xml2_LIBS)" OBJDUMP=objdump
BUILD_ACTION=$(GNUMAKE)
BUILD_FLAGS+= -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)
.ELSE
CONFIGURE_DIR=win32
CONFIGURE_ACTION=cscript configure.js
CONFIGURE_FLAGS=iconv=no sax1=yes
BUILD_ACTION=nmake
BUILD_DIR=$(CONFIGURE_DIR)
.ENDIF
.ELSE

.IF "$(debug)" != ""
xml2_CFLAGS+=-g
.ELSE
xml2_CFLAGS+=-O
.ENDIF

xml2_CFLAGS+=$(ARCH_FLAGS)

.IF "$(SYSBASE)"!=""
xml2_CFLAGS+=-I$(SYSBASE)$/usr$/include 
.IF "$(COMNAME)"=="sunpro5"
xml2_CFLAGS+=$(C_RESTRICTIONFLAGS)
.ENDIF			# "$(COMNAME)"=="sunpro5"
xml2_LDFLAGS+=-L$(SYSBASE)$/usr$/lib
.ENDIF			# "$(SYSBASE)"!=""

CONFIGURE_DIR=
CONFIGURE_ACTION=.$/configure
.IF "$(DISABLE_DYNLOADING)" == "TRUE"
CONFIGURE_FLAGS=--disable-shared
.ELSE
CONFIGURE_FLAGS=--disable-static
.ENDIF
CONFIGURE_FLAGS+=--enable-ipv6=no --without-python --without-zlib --with-sax1=yes CFLAGS="$(xml2_CFLAGS) $(EXTRA_CFLAGS)" LDFLAGS="$(xml2_LDFLAGS) $(EXTRA_LINKFLAGS)"
BUILD_ACTION=$(GNUMAKE)
BUILD_FLAGS+= -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)
.IF "$(debug)"!=""
CONFIGURE_FLAGS+=--with-mem-debug --with-run-debug
.ENDIF
.IF "$(OS)" == "MACOSX"
CONFIGURE_FLAGS += \
    --prefix=/@.__________________________________________________$(EXTRPATH)
.END
.IF "$(CROSS_COMPILING)"=="YES"
CONFIGURE_FLAGS+=--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)
.ENDIF
.ENDIF


OUTDIR2INC=include$/libxml 

.IF "$(OS)"=="MACOSX"
EXTRPATH=URELIB
OUT2LIB+=.libs$/libxml2.*.dylib
OUT2BIN_NONE+=.libs$/xmllint
OUT2BIN+=xml2-config
.ELIF "$(DISABLE_DYNLOADING)" == "TRUE"
OUT2LIB+=.libs$/libxml2.a
OUT2BIN+=xml2-config
.ELIF "$(OS)"=="WNT"
.IF "$(COM)"=="GCC"
OUT2LIB+=.libs$/libxml2*.a
OUT2BIN+=.libs$/xmllint.exe
OUT2BIN+=xml2-config
.ELSE
OUT2LIB+=win32$/bin.msvc$/*.lib
OUT2BIN+=win32$/bin.msvc$/*.dll
OUT2BIN+=win32$/bin.msvc$/xmllint.exe
.ENDIF
.ELSE
OUT2LIB+=.libs$/libxml2.so*
OUT2BIN+=.libs$/xmllint
OUT2BIN+=xml2-config
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

