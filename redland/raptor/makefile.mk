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
TARGET=so_raptor

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(SYSTEM_REDLAND)" == "YES"
all:
	@echo "An already available installation of Redland RDF should exist on your system."
	@echo "Therefore the version provided here does not need to be built in addition."
.ENDIF

# --- Files --------------------------------------------------------

.INCLUDE :	../redlandversion.mk

RAPTORVERSION=$(RAPTOR_MAJOR).4.18

TARFILE_NAME=raptor-$(RAPTORVERSION)
TARFILE_MD5=284e768eeda0e2898b0d5bf7e26a016e

ADDITIONAL_FILES=src/makefile.mk src/raptor_config.h

OOO_PATCH_FILES= \
    $(TARFILE_NAME).patch.legal \
    $(TARFILE_NAME).patch.autotools \
    $(TARFILE_NAME).patch.ooo_build \
    $(TARFILE_NAME).patch.dmake \
    $(TARFILE_NAME).patch.win32 \
    $(TARFILE_NAME).patch.fixes \
    $(TARFILE_NAME).patch.rindex \
    raptor-aix.patch \
    $(TARFILE_NAME).entities.patch

.IF "$(GUI)" == "UNX"
OOO_PATCH_FILES+= \
    $(TARFILE_NAME).patch.bundled-soname
.ENDIF

.IF "$(CROSS_COMPILING)"=="YES"
OOO_PATCH_FILES += \
    $(TARFILE_NAME).patch.cross
.ENDIF

PATCH_FILES=$(OOO_PATCH_FILES)

.IF "$(SYSTEM_LIBXML)" == "YES"
PATCH_FILES+=raptor-1.4.18.libxml.patch
.ENDIF
.IF "$(SYSTEM_LIBXSLT)" == "YES"
PATCH_FILES+=raptor-1.4.18.libxslt.patch
.ENDIF

.IF "$(OS)"=="ANDROID"
PATCH_FILES+=$(TARFILE_NAME).patch.no-soname
.ENDIF

.IF "$(OS)"=="WNT"
.IF "$(COM)"=="GCC"
OOO_PATCH_FILES+=$(TARFILE_NAME).patch.mingw
raptor_CC=$(CC) -mthreads
.IF "$(MINGW_SHARED_GCCLIB)"=="YES"
raptor_CC+=-shared-libgcc
.ENDIF
raptor_LIBS=

raptor_LDFLAGS=-Wl,--no-undefined -Wl,--enable-runtime-pseudo-reloc-v2,--export-all-symbols
.IF "$(ILIB)"!=""
raptor_LDFLAGS+= -L$(ILIB:s/;/ -L/)
.ENDIF

CONFIGURE_DIR=
CONFIGURE_ACTION=.$/configure
# do not enable grddl parser (#i93768#)
CONFIGURE_FLAGS=--disable-static --enable-shared --disable-gtk-doc --with-openssl-digests --with-xml-parser=libxml --enable-parsers="rdfxml ntriples turtle trig guess rss-tag-soup" --without-bdb --without-sqlite --without-mysql --without-postgresql --without-threestore       --with-regex-library=posix --with-decimal=none --with-www=xml --build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM) --target=$(HOST_PLATFORM) lt_cv_cc_dll_switch="-shared" CC="$(raptor_CC)" CPPFLAGS="$(INCLUDE)" LDFLAGS="$(raptor_LDFLAGS)" LIBS="$(raptor_LIBS)" OBJDUMP="$(WRAPCMD) $(HOST_PLATFORM)-objdump" LIBXML2LIB="$(LIBXML2LIB)" XSLTLIB="$(XSLTLIB)"
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

.IF "$(debug)" != ""
CFLAGS:=-g
.ELSE
CFLAGS:=-O
.ENDIF
.IF "$(COM)"=="C52" && "$(CPU)"=="U"
CFLAGS+=-m64
.ENDIF
#ppc64 ld crashing at link time, throwing a minimal-toc at it to
#perturb is sufficiently to not crash
.IF "$(COM)$(OS)$(CPUNAME)" == "GCCLINUXPOWERPC64"
CFLAGS+=-mminimal-toc
.ENDIF
.EXPORT: CFLAGS

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

CPPFLAGS+:=$(EXTRA_CDEFS) $(EXTRA_CFLAGS)
LDFLAGS+:=$(EXTRA_LINKFLAGS)
XSLTLIB!:=$(XSLTLIB) # expand dmake variables for xslt-config

.EXPORT: CPPFLAGS
.EXPORT: LDFLAGS
.EXPORT: LIBXML2LIB
.EXPORT: XSLTLIB

CONFIGURE_DIR=
CONFIGURE_ACTION=.$/configure

.IF "$(OS)"=="IOS"
CONFIGURE_ACTION+=LIBS=-liconv
.ENDIF

.IF "$(OS)"=="IOS" || "$(OS)"=="ANDROID"
CONFIGURE_FLAGS=--disable-shared
.ELSE
CONFIGURE_FLAGS=--disable-static
.ENDIF

# do not enable grddl parser (#i93768#)
CONFIGURE_FLAGS+= --disable-gtk-doc --with-threads --with-openssl-digests --with-xml-parser=libxml --enable-parsers="rdfxml ntriples turtle trig guess rss-tag-soup" --without-bdb --without-sqlite --without-mysql --without-postgresql --without-threestore       --with-regex-library=posix --with-decimal=none --with-www=xml

.IF "$(SYSTEM_LIBXML)" != "YES"
CONFIGURE_FLAGS+=--with-xml2-config=$(SOLARVER)/$(INPATH)/bin/xml2-config
.ENDIF

.IF "$(SYSTEM_LIBXSLT)" != "YES"
CONFIGURE_FLAGS+=--with-xslt-config=$(SOLARVER)/$(INPATH)/bin/xslt-config
.ENDIF

.IF "$(OS)" == "MACOSX"
CONFIGURE_FLAGS += \
    --prefix=/@.__________________________________________________$(EXTRPATH)
.END

.IF "$(CROSS_COMPILING)"=="YES"
CONFIGURE_FLAGS+= --build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)
.ENDIF
BUILD_ACTION=$(GNUMAKE)
BUILD_FLAGS+= -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)
.ENDIF


OUT2INC+=src$/raptor.h

.IF "$(OS)"=="MACOSX"
OUT2LIB+=src$/.libs$/libraptor-lo.$(RAPTOR_MAJOR).dylib src$/.libs$/libraptor.dylib
OUT2BIN+=src/raptor-config
.ELIF "$(OS)"=="IOS" || "$(OS)"=="ANDROID"
OUT2LIB+=src$/.libs$/libraptor.a
OUT2BIN+=src/raptor-config
.ELIF "$(OS)"=="AIX"
OUT2LIB+=src$/.libs$/libraptor-lo.so.$(RAPTOR_MAJOR) src$/.libs$/libraptor.so
OUT2BIN+=src/raptor-config
.ELIF "$(OS)"=="WNT"
.IF "$(COM)"=="GCC"
OUT2LIB+=src$/.libs$/*.a
OUT2BIN+=src$/.libs$/*.dll
OUT2BIN+=src/raptor-config
.ELSE
# if we use dmake, this is done automagically
.ENDIF
.ELSE
OUT2LIB+=src$/.libs$/libraptor-lo.so.$(RAPTOR_MAJOR) src$/.libs$/libraptor.so
OUT2BIN+=src/raptor-config
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

