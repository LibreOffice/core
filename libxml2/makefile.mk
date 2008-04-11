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
# $Revision: 1.21 $
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

LIBXML2VERSION=2.6.17

TARFILE_NAME=$(PRJNAME)-$(LIBXML2VERSION)
.IF "$(OS)$(COM)"=="WNTGCC"
PATCH_FILE_NAME=$(TARFILE_NAME)-mingw.patch
.ELSE
PATCH_FILE_NAME=$(TARFILE_NAME).patch
.ENDIF

# This is only for UNX environment now

.IF "$(OS)"=="WNT"
.IF "$(COM)"=="GCC"
CONFIGURE_DIR=
CONFIGURE_ACTION=.$/configure
CONFIGURE_FLAGS=--enable-ipv6=no --without-python --enable-static=no --without-debug --build=i586-pc-mingw32 --host=i586-pc-mingw32 lt_cv_cc_dll_switch="-shared" CFLAGS=-D_MT LDFLAGS="-no-undefined -Wl,--enable-runtime-pseudo-reloc -L$(ILIB:s/;/ -L/)" LIBS="-lws2_32 -lmingwthrd" OBJDUMP="$(WRAPCMD) objdump"
BUILD_ACTION=$(GNUMAKE)
BUILD_DIR=$(CONFIGURE_DIR)
.ELSE
CONFIGURE_DIR=win32
CONFIGURE_ACTION=cscript configure.js
CONFIGURE_FLAGS=iconv=no sax1=yes
.IF "$(debug)"!=""
CONFIGURE_FLAGS+=debug=yes
.ENDIF
BUILD_ACTION=nmake
BUILD_DIR=$(CONFIGURE_DIR)
.ENDIF
.ELSE
.IF "$(SYSBASE)"!=""
xml2_CFLAGS+=-I$(SYSBASE)$/usr$/include 
.IF "$(COMNAME)"=="sunpro5"
xml2_CFLAGS+=$(C_RESTRICTIONFLAGS)
.ENDIF			# "$(COMNAME)"=="sunpro5"
xml2_LDFLAGS+=-L$(SYSBASE)$/usr$/lib
.ENDIF			# "$(SYSBASE)"!=""

.IF "$(OS)$(COM)"=="LINUXGCC"
xml2_LDFLAGS+=-Wl,-z,noexecstack
.ENDIF

CONFIGURE_DIR=
CONFIGURE_ACTION=.$/configure
CONFIGURE_FLAGS=--enable-ipv6=no --without-python --enable-static=no --with-sax1=yes ADDCFLAGS="$(xml2_CFLAGS) $(EXTRA_CFLAGS)" LDFLAGS="$(xml2_LDFLAGS) $(EXTRA_LINKFLAGS)"
BUILD_ACTION=$(GNUMAKE)
BUILD_FLAGS+= -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)
.ENDIF


OUTDIR2INC=include$/libxml 

.IF "$(OS)"=="MACOSX"
EXTRPATH=URELIB
OUT2LIB+=.libs$/libxml2.*.dylib
OUT2BIN+=.libs$/xmllint
.ELIF "$(OS)"=="WNT"
.IF "$(COM)"=="GCC"
OUT2LIB+=.libs$/libxml2*.a
OUT2BIN+=.libs$/libxml2*.dll
OUT2BIN+=.libs$/xmllint.exe
.ELSE
OUT2LIB+=win32$/bin.msvc$/*.lib
OUT2BIN+=win32$/bin.msvc$/*.dll
OUT2BIN+=win32$/bin.msvc$/xmllint.exe
.ENDIF
.ELSE
OUT2LIB+=.libs$/libxml2.so*
OUT2BIN+=.libs$/xmllint
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

