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

RASQALVERSION=0.9.33

TARFILE_NAME=rasqal-$(RASQALVERSION)
TARFILE_MD5=1f5def51ca0026cd192958ef07228b52

ADDITIONAL_FILES=src/makefile.mk

OOO_PATCH_FILES= \
    $(TARFILE_NAME).patch.ooo_build \
    $(TARFILE_NAME).patch.dmake \

PATCH_FILES=$(OOO_PATCH_FILES)


.IF "$(OS)"=="OS2"
CONFIGURE_DIR=
CONFIGURE_ACTION=libtoolize && aclocal && autoconf && .$/configure RAPTOR2_CFLAGS=-I${PWD}$/..$/${INPATH}/inc RAPTOR2_LIBS='-L${PWD}$/..$/${INPATH}/lib -lraptor2 -lxml2 -lcurl' PKG_CONFIG_PATH='../raptor2-2.0.15;../rasqal-0.9.33'
CONFIGURE_FLAGS=--disable-static --disable-gtk-doc --with-regex-library=posix --with-decimal=none
BUILD_ACTION=$(GNUMAKE)
BUILD_FLAGS+= -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)
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
OOO_PATCH_FILES+= $(TARFILE_NAME).patch.win32
CONFIGURE_ACTION= ${PERL} -p -e '$$prefix="'$(PWD)/$(OUT)'"; $$libdir="'$(PWD)/$(LB)'"; $$incdir="'$(PWD)/$(INCCOM)'"; $$ldflags="-L$(SOLARLIBDIR) -lraptor2 -lm";' -e 's/\@prefix\@/$$prefix/; s/\@exec_prefix\@/\$${prefix}/; s/\@libdir\@/$$libdir/; s/\@includedir\@/$$incdir/; s/\@PACKAGE\@/rasqal/; s/\@VERSION\@/0.9.33/; s/\@PKG_CONFIG_REQUIRES\@/raptor2 >= 2.0.7/; s/\@RAPTOR_LDFLAGS\@/$$ldflags/;'  < rasqal.pc.in > rasqal.pc && \
    ${PERL} -p -e 's/\@RASQAL_VERSION_DECIMAL\@/933/;s/\@VERSION\@/0.9.33/;s/\@RASQAL_VERSION_MAJOR\@/0/;s/\@RASQAL_VERSION_MINOR\@/9/;s/\@RASQAL_VERSION_RELEASE\@/33/;s/\@HAVE_SYS_TIME_H\@/0/;s/\@HAVE_TIME_H\@/1/;s/\@RAPTOR_VERSION_DEC\@/2.0.15/' < src/rasqal.h.in > src/rasqal.h && \
    $(COPY) src/rasqal_config.h.in src/rasqal_config.h && \
    $(COPY) src/snprintf.c src/rasqal_snprintf.c && \
    $(COPY) libsv/sv.c src/sv.c
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
CONFIGURE_ACTION=.$/configure PATH="..$/..$/..$/bin:$$PATH" RASQAL_CFLAGS=-I${PWD}$/..$/${INPATH}/inc RASQAL_LIBS="-L${PWD}/..$/${INPATH}/lib -lrasqual" RAPTOR2_CFLAGS=-I${PWD}$/..$/${INPATH}/inc RAPTOR2_LIBS="-L${PWD}/..$/${INPATH}/lib -lraptor2" PKG_CONFIG_PATH="../raptor2-2.0.15:../rasqal-0.9.33"
CONFIGURE_FLAGS=--disable-static --disable-gtk-doc --with-regex-library=posix --with-decimal=none
.IF "$(SYSTEM_LIBXML)" == "NO"
CONFIGURE_FLAGS+=--with-xml2-config=${SOLARVERSION}/${INPATH}/bin/xml2-config
.ENDIF
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
OUT2LIB+=src$/.libs$/*.a
OUT2BIN+=src$/.libs$/*.dll
.ELSE
OUT2LIB+=src$/.libs$/librasqal.so.$(RASQAL_MAJOR) src$/.libs$/librasqal.so
OUT2BIN+=src/rasqal-config
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

