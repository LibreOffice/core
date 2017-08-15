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

RAPTORVERSION=2.0.15

TARFILE_NAME=raptor2-$(RAPTORVERSION)
TARFILE_MD5=a39f6c07ddb20d7dd2ff1f95fa21e2cd

ADDITIONAL_FILES=src/makefile.mk src/raptor_config.h

OOO_PATCH_FILES= \
    $(TARFILE_NAME).patch.dmake \

PATCH_FILES=$(OOO_PATCH_FILES)


.IF "$(OS)"=="OS2"
BUILD_ACTION=dmake
BUILD_DIR=$(CONFIGURE_DIR)/src
ADDITIONAL_FILES+=src/windows.h
OOO_PATCH_FILES+=$(TARFILE_NAME).patch.os2
.ELIF "$(OS)"=="WNT"
.IF "$(COM)"=="GCC"
OOO_PATCH_FILES+=$(TARFILE_NAME).patch.mingw
raptor_CC=$(CC) -mthreads
.IF "$(MINGW_SHARED_GCCLIB)"=="YES"
raptor_CC+=-shared-libgcc
.ENDIF
raptor_LIBS=
.IF "$(MINGW_SHARED_GXXLIB)"=="YES"
raptor_LIBS+=$(MINGW_SHARED_LIBSTDCPP)
.ENDIF

CONFIGURE_DIR=
CONFIGURE_ACTION=.$/configure
# do not enable grddl parser (#i93768#)
CONFIGURE_FLAGS=--disable-static --disable-gtk-doc --with-openssl-digests --enable-parsers="rdfxml ntriples turtle trig guess rss-tag-soup" --without-bdb --without-sqlite --without-mysql --without-postgresql --without-threestore       --with-regex-library=posix --with-decimal=none --with-www=xml --build=i586-pc-mingw32 --host=i586-pc-mingw32 lt_cv_cc_dll_switch="-shared" --prefix=$(PWD)/$(OUT) --includedir=(PWD)$/$(INCCOM) --libdir=$(PWD)$/$(LB) --CC="$(raptor_CC)" CPPFLAGS="-nostdinc $(INCLUDE)" LDFLAGS="-no-undefined -Wl,--enable-runtime-pseudo-reloc-v2,--export-all-symbols  -L$(ILIB:s/;/ -L/)" LIBS="$(raptor_LIBS)" OBJDUMP="$(WRAPCMD) objdump" LIBXML2LIB=$(LIBXML2LIB) XSLTLIB="$(XSLTLIB)"
BUILD_ACTION=$(GNUMAKE)
BUILD_FLAGS+= -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)
.ELSE
# there is no wntmsci build environment in the tarball; we use custom dmakefile
OOO_PATCH_FILES+=$(TARFILE_NAME).patch.win32
CONFIGURE_ACTION= ${PERL} -p -e '$$prefix="'$(PWD)/$(OUT)'"; $$libdir="'$(PWD)/$(LB)'"; $$incdir="'$(PWD)/$(INCCOM)'"; $$ldflags="-L$(SOLARLIBDIR) -lxml2 -lm";' -e 's/\@prefix\@/$$prefix/; s/\@exec_prefix\@/\$${prefix}/; s/\@libdir\@/$$libdir/; s/\@includedir\@/$$incdir/; s/\@PACKAGE\@/raptor2/; s/\@VERSION\@/2.0.15/; s/\@PKG_CONFIG_REQUIRES\@//; s/\@RAPTOR_LDFLAGS\@/$$ldflags/;'  < raptor2.pc.in > raptor2.pc && \
    ${PERL} -p -e 's/\@RAPTOR_VERSION_DECIMAL\@/20015/;s/\@VERSION\@/2.0.15/;s/\@RAPTOR_VERSION_MAJOR\@/2/;s/\@RAPTOR_VERSION_MINOR\@/0/;s/\@RAPTOR_VERSION_RELEASE\@/15/' < src/raptor2.h.in > src/raptor2.h && \
    $(COPY) src/raptor_config.h.in src/raptor_config.h
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
CONFIGURE_ACTION=.$/configure
# do not enable grddl parser (#i93768#)
CONFIGURE_FLAGS=--disable-static --disable-gtk-doc --enable-parsers="rdfxml ntriples turtle trig guess rss-tag-soup" --with-www=xml --prefix=$(PDW)/$(OUT) --includedir=$(PWD)/$(INCCOM) --libdir=$(PWD)/$(LB)
.IF "$(SYSTEM_LIBXML)" == "NO"
CONFIGURE_FLAGS+=--with-xml2-config=${SOLARVERSION}/${INPATH}/bin/xml2-config \
    --with-xslt-config=${SOLARVERSION}/${INPATH}/bin/xslt-config
.ENDIF
BUILD_ACTION=$(GNUMAKE)
BUILD_FLAGS+= -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)
#INSTALL_ACTION=$(GNUMAKE) install
#INSTALL_FLAGS+=DESTDIR=$(PWD)$/$(P_INSTALL_TARGET_DIR)
.ENDIF


OUT2INC+=src/raptor.h src/raptor2.h src/raptor_config.h

.IF "$(OS)"=="MACOSX"
OUT2LIB+=src$/.libs$/libraptor2.$(RAPTOR_MAJOR).dylib src$/.libs$/libraptor2.dylib
.ELIF "$(OS)"=="WNT"
.IF "$(COM)"=="GCC"
OUT2LIB+=src$/.libs$/*.a
OUT2BIN+=src$/.libs$/*.dll
.ELSE
# if we use dmake, this is done automagically
.ENDIF
.ELIF "$(GUI)"=="OS2"
# if we use dmake, this is done automagically
.ELSE
OUT2LIB+=src$/.libs$/libraptor2.so.$(RAPTOR_MAJOR) src$/.libs$/libraptor2.so
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

