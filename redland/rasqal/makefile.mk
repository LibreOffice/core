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
# $Revision: 1.8 $
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

#RASQALVERSION=$(RASQAL_MAJOR).$(RASQAL_MINOR).$(RASQAL_MICRO)
RASQALVERSION=0.9.15

TARFILE_NAME=rasqal-$(RASQALVERSION)
PATCH_FILES=..$/$(TARFILE_NAME).patch

ADDITIONAL_FILES=src/makefile.mk


.IF "$(OS)"=="WNT"
.IF "$(COM)"=="GCC"
CONFIGURE_DIR=
CONFIGURE_ACTION=.$/configure PATH="..$/..$/..$/bin:$$PATH"
CONFIGURE_FLAGS=--disable-static --disable-gtk-doc --with-openssl-digests --with-xml-parser=libxml --without-bdb --without-sqlite --without-mysql --without-postgresql --without-threestore       --disable-pcre --with-decimal=none --with-www=xml --build=i586-pc-mingw32 --host=i586-pc-mingw32 lt_cv_cc_dll_switch="-shared" CFLAGS=-D_MT CPPFLAGS="-nostdinc $(INCLUDE)" LDFLAGS="-no-undefined -Wl,--enable-runtime-pseudo-reloc,--export-all-symbols -L$(ILIB:s/;/ -L/)" LIBS=-lmingwthrd OBJDUMP="$(WRAPCMD) objdump" LIBXML2LIB=$(LIBXML2LIB) ZLIB3RDLIB=$(ZLIB3RDLIB) XSLTLIB="$(XSLTLIB)"
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
LDFLAGS:=-Wl,-rpath,'$$$$ORIGIN:$$$$ORIGIN/../ure-link/lib' -Wl,-noinhibit-exec -Wl,-z,noexecstack
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

.IF "$(SYSBASE)"!=""
CPPFLAGS+:=-I$(SYSBASE)$/usr$/include
.IF "$(OS)"=="SOLARIS" || "$(OS)"=="LINUX"
LDFLAGS+:=-L$(SYSBASE)$/lib -L$(SYSBASE)$/usr$/lib -lpthread -ldl
.ENDIF
.ENDIF			# "$(SYSBASE)"!=""

CPPFLAGS+:=$(EXTRA_CFLAGS)
LDFLAGS+:=$(EXTRA_LINKFLAGS)

.EXPORT: CPPFLAGS
.EXPORT: LDFLAGS
.EXPORT: LIBXML2LIB
.EXPORT: ZLIB3RDLIB

CONFIGURE_DIR=
CONFIGURE_ACTION=.$/configure PATH="..$/..$/..$/bin:$$PATH"
CONFIGURE_FLAGS=--disable-static --disable-gtk-doc --with-threads --with-openssl-digests --with-xml-parser=libxml --without-bdb --without-sqlite --without-mysql --without-postgresql --without-threestore       --with-regex-library=posix --with-decimal=none --with-www=xml
BUILD_ACTION=$(GNUMAKE)
BUILD_FLAGS+= -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)
#INSTALL_ACTION=$(GNUMAKE) install
#INSTALL_FLAGS+=DESTDIR=$(PWD)$/$(P_INSTALL_TARGET_DIR)
.ENDIF


OUT2INC+=src$/rasqal.h

.IF "$(OS)"=="MACOSX"
OUT2LIB+=src$/.libs$/librasqal*.dylib
OUT2BIN+=src/rasqal-config
.ELIF "$(OS)"=="WNT"
.IF "$(COM)"=="GCC"
OUT2LIB+=src$/.libs$/*.a
OUT2BIN+=src$/.libs$/*.dll
OUT2BIN+=src/rasqal-config
.ELSE
# if we use dmake, this is done automagically
.ENDIF

.ELIF "$(OS)"=="OS2"
OUT2LIB+=src$/.libs$/*.a
OUT2BIN+=src$/rasqal-config

.ELSE
OUT2LIB+=src$/.libs$/librasqal*.so*
OUT2BIN+=src/rasqal-config
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

