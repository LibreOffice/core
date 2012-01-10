#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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
    $(TARFILE_NAME).patch.autotools \
    $(TARFILE_NAME).patch.ooo_build \
    $(TARFILE_NAME).patch.dmake \
    $(TARFILE_NAME).patch.win32

PATCH_FILES=$(OOO_PATCH_FILES)


.IF "$(OS)"=="OS2"
BUILD_ACTION=dmake
BUILD_DIR=$(CONFIGURE_DIR)$/src
ADDITIONAL_FILES+=src/windows.h
OOO_PATCH_FILES+=$(TARFILE_NAME).patch.os2
.ELIF "$(OS)"=="WNT"
.IF "$(COM)"=="GCC"
rasqal_CC=$(CC) -mthreads
.IF "$(MINGW_SHARED_GCCLIB)"=="YES"
rasqal_CC+=-shared-libgcc
.ENDIF
rasqal_LIBS=
.IF "$(MINGW_SHARED_GXXLIB)"=="YES"
rasqal_LIBS+=$(MINGW_SHARED_LIBSTDCPP)
.ENDIF

CONFIGURE_DIR=
CONFIGURE_ACTION=.$/configure PATH="..$/..$/..$/bin:$$PATH"
CONFIGURE_FLAGS=--disable-static --disable-gtk-doc --with-openssl-digests --with-xml-parser=libxml --without-bdb --without-sqlite --without-mysql --without-postgresql --without-threestore --disable-pcre --with-decimal=none --with-www=xml --build=i586-pc-mingw32 --host=i586-pc-mingw32 lt_cv_cc_dll_switch="-shared" CC="$(rasqal_CC)" CPPFLAGS="-nostdinc $(INCLUDE)" LDFLAGS="-no-undefined -Wl,--enable-runtime-pseudo-reloc-v2,--export-all-symbols -L$(ILIB:s/;/ -L/)" LIBS="$(rasqal_LIBS)" OBJDUMP="$(WRAPCMD) objdump" LIBXML2LIB=$(LIBXML2LIB) XSLTLIB="$(XSLTLIB)"
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
CPPFLAGS+:=-I$(SOLARINCDIR)$/external
LDFLAGS+:=-L$(SOLARLIBDIR)

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
CONFIGURE_ACTION=.$/configure PATH="..$/..$/..$/bin:$$PATH"
CONFIGURE_FLAGS=--disable-static --disable-gtk-doc --with-threads --with-openssl-digests --with-xml-parser=libxml --without-bdb --without-sqlite --without-mysql --without-postgresql --without-threestore       --with-regex-library=posix --with-decimal=none --with-www=xml
BUILD_ACTION=$(AUGMENT_LIBRARY_PATH) $(GNUMAKE)
BUILD_FLAGS+= -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)
#INSTALL_ACTION=$(GNUMAKE) install
#INSTALL_FLAGS+=DESTDIR=$(PWD)$/$(P_INSTALL_TARGET_DIR)
.ENDIF


OUT2INC+=src$/rasqal.h

.IF "$(OS)"=="MACOSX"
OUT2LIB+=src$/.libs$/librasqal.$(RASQAL_MAJOR).dylib src$/.libs$/librasqal.dylib
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
# if we use dmake, this is done automagically
.ELSE
OUT2LIB+=src$/.libs$/librasqal.so.$(RASQAL_MAJOR) src$/.libs$/librasqal.so
OUT2BIN+=src/rasqal-config
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

