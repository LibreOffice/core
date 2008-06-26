#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: mst $ $Date: 2008-06-26 10:52:14 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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
PATCH_FILE_NAME=$(TARFILE_NAME).patch

ADDITIONAL_FILES=src/makefile.mk


.IF "$(OS)"=="WNT"
.IF "$(COM)"=="GCC"
#CONFIGURE_DIR=
#CONFIGURE_ACTION=.$/configure
#CONFIGURE_FLAGS=--enable-ipv6=no --without-python --enable-static=no --without-debug --build=i586-pc-mingw32 --host=i586-pc-mingw32 lt_cv_cc_dll_switch="-shared" CFLAGS=-D_MT LDFLAGS="-no-undefined -Wl,--enable-runtime-pseudo-reloc -L$(ILIB:s/;/ -L/)" LIBS="-lws2_32 -lmingwthrd" OBJDUMP="$(WRAPCMD) objdump"
#BUILD_ACTION=$(GNUMAKE)
#BUILD_DIR=$(CONFIGURE_DIR)
all:
    @echo "FIXME: somebody port me to mingw please..."
    exit 1
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

.IF "$(SYSBASE)"!=""
CPPFLAGS+=-I$(SOLARINCDIR)$/external -I$(SYSBASE)$/usr$/include
.IF "$(OS)"=="SOLARIS" || "$(OS)"=="LINUX"
# NB: SOLARLIBDIR is first, because linux SYSBASE contains obsolete libcrypto
LDFLAGS+:=-L$(SOLARLIBDIR) -L$(SYSBASE)$/lib -L$(SYSBASE)$/usr$/lib -lpthread -ldl
.ENDIF
.ENDIF			# "$(SYSBASE)"!=""

CPPFLAGS+:=$(EXTRA_CFLAGS)
LDFLAGS+:=$(EXTRA_LINKFLAGS)
XSLTLIB:=$(XSLTLIB)

.EXPORT: CPPFLAGS
.EXPORT: LDFLAGS
.EXPORT: LIBXML2LIB
.EXPORT: ZLIB3RDLIB
.EXPORT: XSLTLIB

CONFIGURE_DIR=
CONFIGURE_ACTION=.$/configure PATH=..$/..$/..$/bin:$$PATH
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
.ELSE
OUT2LIB+=src$/.libs$/librasqal*.so*
OUT2BIN+=src/rasqal-config
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

