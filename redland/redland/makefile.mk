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
TARGET=so_redland

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(SYSTEM_REDLAND)" == "YES"
all:
    @echo "An already available installation of Redland RDF should exist on your system."
    @echo "Therefore the version provided here does not need to be built in addition."
.ENDIF

# --- Files --------------------------------------------------------

.INCLUDE :	../redlandversion.mk

REDLANDVERSION=1.0.8

TARFILE_NAME=redland-$(REDLANDVERSION)
TARFILE_MD5=ca66e26082cab8bb817185a116db809b

ADDITIONAL_FILES=librdf/makefile.mk librdf/rdf_config.h

OOO_PATCH_FILES= \
    $(TARFILE_NAME).patch.legal \
    $(TARFILE_NAME).patch.autotools \
    $(TARFILE_NAME).patch.dmake \
    $(TARFILE_NAME).patch.ooo_build \
    $(TARFILE_NAME).patch.win32 \
    redland-aix.patch

PATCH_FILES=$(OOO_PATCH_FILES) \


.IF "$(OS)"=="OS2"
BUILD_ACTION=dmake
BUILD_DIR=$(CONFIGURE_DIR)$/librdf
.ELIF "$(OS)"=="WNT"
.IF "$(COM)"=="GCC"
redland_CC=$(CC) -mthreads
.IF "$(MINGW_SHARED_GCCLIB)"=="YES"
redland_CC+=-shared-libgcc
.ENDIF
redland_LIBS=
.IF "$(MINGW_SHARED_GXXLIB)"=="YES"
redland_LIBS+=$(MINGW_SHARED_LIBSTDCPP)
.ENDIF

CONFIGURE_DIR=
CONFIGURE_ACTION=.$/configure PATH="..$/..$/..$/bin:$$PATH"
CONFIGURE_FLAGS=--disable-static --disable-gtk-doc --with-openssl-digests --with-xml-parser=libxml --with-raptor=system --with-rasqual=system --without-bdb --without-sqlite --without-mysql --without-postgresql --without-threestore --with-regex-library=posix --with-decimal=none --with-www=xml --build=i586-pc-mingw32 --host=i586-pc-mingw32 lt_cv_cc_dll_switch="-shared" CC="$(redland_CC)" CPPFLAGS="-nostdinc $(INCLUDE)" LDFLAGS="-no-undefined -Wl,--enable-runtime-pseudo-reloc-v2,--export-all-symbols -L$(ILIB:s/;/ -L/)" LIBS="$(redland_LIBS)" OBJDUMP="$(WRAPCMD) objdump" LIBXML2LIB=$(LIBXML2LIB) XSLTLIB="$(XSLTLIB)"
BUILD_ACTION=$(GNUMAKE)
BUILD_FLAGS+= -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)
.ELSE
# there is no wntmsci build environment in the tarball; we use custom dmakefile
BUILD_ACTION=dmake
BUILD_DIR=$(CONFIGURE_DIR)$/librdf
.ENDIF
.ELSE # "WNT"

.IF "$(OS)$(COM)"=="LINUXGCC" || "$(OS)$(COM)"=="FREEBSDGCC"
LDFLAGS:=-Wl,-rpath,'$$$$ORIGIN:$$$$ORIGIN/../ure-link/lib' -Wl,-noinhibit-exec
.ENDIF                  # "$(OS)$(COM)"=="LINUXGCC"
.IF "$(OS)$(COM)"=="SOLARISC52"
LDFLAGS:=-Wl,-R'$$$$ORIGIN:$$$$ORIGIN/../ure-link/lib'
.ENDIF                  # "$(OS)$(COM)"=="SOLARISC52"

.IF "$(COM)"=="C52" && "$(CPU)"=="U"
CFLAGS=-m64
.EXPORT: CFLAGS
.ENDIF

# NB: SOLARDIR before SYSBASE, because linux SYSBASE contains obsolete libcrypto
CPPFLAGS+:=-I$(PWD)$/$(INCCOM) -I$(SOLARINCDIR)$/external
LDFLAGS+:=-L$(PWD)$/$(LB) -L$(SOLARLIBDIR)

.IF "$(OS)"=="AIX"
LDFLAGS+:=$(LINKFLAGS) $(LINKFLAGSRUNPATH_OOO)
.ENDIF

.IF "$(SYSBASE)"!=""
CPPFLAGS+:=-I$(SYSBASE)$/usr$/include
.IF "$(OS)"=="SOLARIS" || "$(OS)"=="LINUX"
LDFLAGS+:=-L$(SYSBASE)$/lib -L$(SYSBASE)$/usr$/lib -lpthread -ldl
.ENDIF
.ENDIF			# "$(SYSBASE)"!=""

.IF "$(OS)"=="MACOSX"
LDFLAGS+:=-Wl,-dylib_file,@loader_path/libraptor.1.dylib:$(PWD)/$(LB)/libraptor.1.dylib
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
CONFIGURE_FLAGS=--disable-static --disable-gtk-doc --with-threads --with-openssl-digests --with-xml-parser=libxml --with-raptor=system --with-rasqual=system --without-bdb --without-sqlite --without-mysql --without-postgresql --without-threestore       --with-regex-library=posix --with-decimal=none --with-www=xml
BUILD_ACTION=$(AUGMENT_LIBRARY_PATH) $(GNUMAKE)
BUILD_FLAGS+= -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)
.ENDIF


OUT2INC+=librdf$/*.h

.IF "$(OS)"=="MACOSX"
OUT2LIB+=librdf$/.libs$/librdf.$(REDLAND_MAJOR).dylib
.ELIF "$(OS)"=="WNT"
.IF "$(COM)"=="GCC"
OUT2LIB+=librdf$/.libs$/*.a
OUT2BIN+=librdf$/.libs$/*.dll
.ELSE
# if we use dmake, this is done automagically
.ENDIF
.ELIF "$(OS)"=="OS2"
# if we use dmake, this is done automagically
.ELSE
OUT2LIB+=librdf$/.libs$/librdf.so.$(REDLAND_MAJOR)
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

