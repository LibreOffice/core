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

PRJNAME=redland
TARGET=so_rasqal

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(SYSTEM_REDLAND)" == "YES"
all:
    @echo "An already available installation of Redland RDF should exist on your system."
    @echo "Therefore the version provided here does not need to be built in addition."
.ENDIF

# --- Files --------------------------------------------------------

.INCLUDE :	../redlandversion.mk

RASQALVERSION=0.9.16

TARFILE_NAME=rasqal-$(RASQALVERSION)
TARFILE_MD5=fca8706f2c4619e2fa3f8f42f8fc1e9d

ADDITIONAL_FILES=src/makefile.mk src/rasqal_config.h

OOO_PATCH_FILES= \
    $(TARFILE_NAME).patch.legal \
    $(TARFILE_NAME).patch.autotools \
    $(TARFILE_NAME).patch.ooo_build \
    $(TARFILE_NAME).patch.dmake \
    $(TARFILE_NAME).patch.win32 \
    rasqal-aix.patch

.IF "$(GUI)" == "UNX"
OOO_PATCH_FILES+= \
    $(TARFILE_NAME).patch.bundled-soname
.ENDIF

PATCH_FILES=$(OOO_PATCH_FILES)

.IF "$(OS)"=="ANDROID"
PATCH_FILES+=$(TARFILE_NAME).patch.no-soname
.ENDIF

.IF "$(OS)"=="WNT"
.IF "$(COM)"=="GCC"
OOO_PATCH_FILES+=$(TARFILE_NAME).patch.mingw
rasqal_CC=$(CC) -mthreads
.IF "$(MINGW_SHARED_GCCLIB)"=="YES"
rasqal_CC+=-shared-libgcc
.ENDIF
rasqal_LIBS=

rasqal_LDFLAGS=-Wl,--no-undefined -Wl,--enable-runtime-pseudo-reloc-v2,--export-all-symbols
.IF "$(ILIB)"!=""
rasqal_LDFLAGS+= -L$(ILIB:s/;/ -L/)
.ENDIF

CONFIGURE_DIR=
CONFIGURE_ACTION=.$/configure PATH="..$/..$/..$/bin:$$PATH"
CONFIGURE_FLAGS=--disable-static --enable-shared --disable-gtk-doc --with-openssl-digests --with-xml-parser=libxml --without-bdb --without-sqlite --without-mysql --without-postgresql --without-threestore --disable-pcre --with-decimal=none --with-www=xml --build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM) lt_cv_cc_dll_switch="-shared" CC="$(rasqal_CC)" CPPFLAGS="$(INCLUDE)" LDFLAGS="$(rasqal_LDFLAGS)" LIBS="$(rasqal_LIBS)" OBJDUMP="$(WRAPCMD) $(HOST_PLATFORM)-objdump" LIBXML2LIB="$(LIBXML2LIB)" XSLTLIB="$(XSLTLIB)"
BUILD_ACTION=$(GNUMAKE)
BUILD_FLAGS+= -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)
.ELSE
# there is no wntmsci build environment in the tarball; we use custom dmakefile
BUILD_ACTION=dmake
BUILD_DIR=$(CONFIGURE_DIR)$/src
.ENDIF
.ELSE # "WNT"

.IF "$(OS)$(COM)"=="LINUXGCC" || "$(OS)$(COM)"=="FREEBSDGCC"
LDFLAGS:=-Wl,-z,origin -Wl,-rpath,'$$$$ORIGIN:$$$$ORIGIN/../ure-link/lib' -Wl,-noinhibit-exec
.ENDIF                  # "$(OS)$(COM)"=="LINUXGCC"
.IF "$(OS)$(COM)"=="SOLARISC52"
LDFLAGS:=-Wl,-R'$$$$ORIGIN:$$$$ORIGIN/../ure-link/lib'
.ENDIF                  # "$(OS)$(COM)"=="SOLARISC52"

.IF "$(COM)"=="C52" && "$(CPU)"=="U"
CFLAGS=-m64
.EXPORT: CFLAGS
.ENDIF

# NB: SOLARDIR before SYSBASE, because linux SYSBASE contains obsolete libcrypto
CPPFLAGS+:=-I$(SOLARINCDIR)$/external
LDFLAGS+:=-L$(SOLARLIBDIR)

.IF "$(OS)"=="AIX"
LDFLAGS+:=$(LINKFLAGS) $(LINKFLAGSRUNPATH_OOO)
.ENDIF

.IF "$(SYSBASE)"!=""
CPPFLAGS+:=-I$(SYSBASE)$/usr$/include
.IF "$(OS)"=="SOLARIS" || "$(OS)"=="LINUX"
LDFLAGS+:=-L$(SYSBASE)$/lib -L$(SYSBASE)$/usr$/lib -lpthread -ldl
.ENDIF
.ENDIF			# "$(SYSBASE)"!=""

.IF "$(OS)"=="MACOSX" && "$(SYSTEM_LIBXML)" != "YES"
LDFLAGS+:=-Wl,-dylib_file,@loader_path/../ure-link/lib/libxml2.2.dylib:$(SOLARLIBDIR)/libxml2.2.dylib
.ENDIF

CPPFLAGS+:=$(EXTRA_CDEFS) $(EXTRA_CFLAGS)
LDFLAGS+:=$(EXTRA_LINKFLAGS)
XSLTLIB!:=$(XSLTLIB) # expand dmake variables for xslt-config

.EXPORT: CPPFLAGS
.EXPORT: LDFLAGS
.EXPORT: LIBXML2LIB
.EXPORT: XSLTLIB

CONFIGURE_DIR=
CONFIGURE_ACTION=.$/configure PATH="..$/..$/..$/bin:$$PATH"
.IF "$(OS)"=="IOS" || "$(OS)"=="ANDROID"
CONFIGURE_FLAGS=--disable-shared
.ELSE
CONFIGURE_FLAGS=--disable-static
.ENDIF
CONFIGURE_FLAGS+= --disable-gtk-doc --with-threads --with-openssl-digests --with-xml-parser=libxml --without-bdb --without-sqlite --without-mysql --without-postgresql --without-threestore       --with-regex-library=posix --with-decimal=none --with-www=xml
.IF "$(OS)" == "MACOSX"
CONFIGURE_FLAGS += \
    --prefix=/@.__________________________________________________$(EXTRPATH)
.END
.IF "$(CROSS_COMPILING)"=="YES"
CONFIGURE_FLAGS+= --build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)
.ENDIF
BUILD_ACTION=$(AUGMENT_LIBRARY_PATH) $(GNUMAKE)
BUILD_FLAGS+= -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)
.ENDIF


OUT2INC+=src$/rasqal.h

.IF "$(OS)"=="MACOSX"
OUT2LIB+=src$/.libs$/librasqal-lo.$(RASQAL_MAJOR).dylib src$/.libs$/librasqal.dylib
OUT2BIN+=src/rasqal-config
.ELIF "$(OS)"=="IOS" || "$(OS)"=="ANDROID"
OUT2LIB+=src$/.libs$/librasqal.a
OUT2BIN+=src/rasqal-config
.ELIF "$(OS)"=="WNT"
.IF "$(COM)"=="GCC"
OUT2LIB+=src$/.libs$/*.a
OUT2BIN+=src$/.libs$/*.dll
OUT2BIN+=src/rasqal-config
.ELSE
# if we use dmake, this is done automagically
.ENDIF
.ELSE
OUT2LIB+=src$/.libs$/librasqal-lo.so.$(RASQAL_MAJOR) src$/.libs$/librasqal.so
OUT2BIN+=src/rasqal-config
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

