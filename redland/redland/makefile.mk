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

REDLANDVERSION=1.0.17

TARFILE_NAME=redland-$(REDLANDVERSION)
TARFILE_MD5=e5be03eda13ef68aabab6e42aa67715e

ADDITIONAL_FILES=src/makefile.mk src/rdf_config.h

OOO_PATCH_FILES= \
    $(TARFILE_NAME).patch.autotools \
    $(TARFILE_NAME).patch.dmake \
    $(TARFILE_NAME).patch.ooo_build \

PATCH_FILES=$(OOO_PATCH_FILES) \


.IF "$(OS)"=="OS2"
OOO_PATCH_FILES+=$(TARFILE_NAME).patch.os2
CONFIGURE_DIR=
CONFIGURE_ACTION=libtoolize --force && aclocal && autoconf && .$/configure RAPTOR2_CFLAGS=-I${PWD}$/..$/${INPATH}/inc RAPTOR2_LIBS='-L${PWD}/..$/${INPATH}/lib -lraptor2 -lxml2 -lcurl' PKG_CONFIG_PATH='../raptor2-2.0.15;../rasqal-0.9.33'
CONFIGURE_FLAGS=--disable-dependency-tracking --disable-static --disable-gtk-doc --with-threads --with-openssl-digests --with-xml-parser=libxml --with-raptor=system --with-rasqual=system --without-bdb --without-sqlite --without-mysql --without-postgresql --without-threestore -without-iodbc --without-unixodbc --without-datadirect --without-virtuoso --with-regex-library=posix --with-decimal=none --with-www=xml --disable-ltdl-install --disable-modular --without-included-ltdl --disable-ltdl-convenience
BUILD_ACTION=$(GNUMAKE)
BUILD_FLAGS+= -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)
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
OOO_PATCH_FILES+= $(TARFILE_NAME).patch.win32
CONFIGURE_ACTION= \
    $(COPY) src/rdf_config.h.in src/rdf_config.h
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
CPPFLAGS+:=-I$(PWD)$/$(INCCOM) -I$(SOLARINCDIR)$/external
LDFLAGS+:=-L$(PWD)$/$(LB) -L$(SOLARLIBDIR)

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
#CONFIGURE_ACTION=.$/configure PATH="..$/..$/..$/bin:$$PATH" RAPTOR2_CFLAGS=-I${PWD}$/..$/${INPATH}/inc RAPTOR2_LIBS="-L${PWD}/..$/${INPATH}/lib -lraptor2" PKG_CONFIG_PATH="../raptor2-2.0.15:../rasqal-0.9.33"
CONFIGURE_ACTION=.$/configure PATH="..$/..$/..$/bin:$$PATH" PKG_CONFIG_PATH="../raptor2-2.0.15:../rasqal-0.9.33"
CONFIGURE_FLAGS=--disable-static --disable-gtk-doc --with-threads --with-openssl-digests --with-xml-parser=libxml --with-raptor=system --with-rasqual=system --without-bdb --without-sqlite --without-mysql --without-postgresql --without-threestore -without-iodbc --without-unixodbc --without-datadirect --without-virtuoso --with-regex-library=posix --with-decimal=none --with-www=xml --disable-ltdl-install --disable-modular --without-included-ltdl --disable-ltdl-convenience
BUILD_ACTION=$(AUGMENT_LIBRARY_PATH) $(GNUMAKE)
BUILD_FLAGS+= -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)
#INSTALL_ACTION=$(GNUMAKE) install
#INSTALL_FLAGS+=DESTDIR=$(PWD)$/$(P_INSTALL_TARGET_DIR)
.ENDIF


OUT2INC+=src$/librdf.h src$/redland.h \
    src$/rdf_concepts.h \
    src$/rdf_digest.h \
    src$/rdf_hash.h \
    src$/rdf_init.h \
    src$/rdf_iterator.h \
    src$/rdf_list.h \
    src$/rdf_log.h \
    src$/rdf_model.h \
    src$/rdf_node.h \
    src$/rdf_parser.h \
    src$/rdf_query.h \
    src$/rdf_raptor.h \
    src$/rdf_serializer.h \
    src$/rdf_statement.h \
    src$/rdf_storage.h \
    src$/rdf_storage_module.h \
    src$/rdf_stream.h \
    src$/rdf_uri.h \
    src$/rdf_utf8.h \

.IF "$(OS)"=="MACOSX"
OUT2LIB+=src$/.libs$/librdf.$(REDLAND_MAJOR).dylib
.ELIF "$(OS)"=="WNT"
.IF "$(COM)"=="GCC"
OUT2LIB+=src$/.libs$/*.a
OUT2BIN+=src$/.libs$/*.dll
.ELSE
# if we use dmake, this is done automagically
.ENDIF
.ELIF "$(OS)"=="OS2"
OUT2LIB+=src$/.libs$/*.a
OUT2BIN+=src$/.libs$/*.dll
.ELSE
OUT2LIB+=src$/.libs$/librdf.so.$(REDLAND_MAJOR)
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

