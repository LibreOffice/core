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

LIBXML2VERSION=2.7.8

TARFILE_NAME=$(PRJNAME)-$(LIBXML2VERSION)
TARFILE_MD5=8127a65e8c3b08856093099b52599c86

# libxml2-global-symbols: #i112480#: Solaris ld won't export non-listed symbols
PATCH_FILES=libxml2-configure.patch \
            libxml2-mingw.patch \
            libxml2-fixes.patch	\
            libxml2-global-symbols.patch \
            libxml2-testapi.patch \
            libxml2-runtest.patch

.IF "$(OS)" == "WNT"
PATCH_FILES+= libxml2-long-path.patch
.ENDIF

# This is only for UNX environment now

.IF "$(OS)"=="WNT"
.IF "$(COM)"=="GCC"
xml2_CC=$(CC) -mthreads
.IF "$(MINGW_SHARED_GCCLIB)"=="YES"
xml2_CC+=-shared-libgcc
.ENDIF
xml2_LIBS=-lws2_32
.IF "$(MINGW_SHARED_GXXLIB)"=="YES"
xml2_LIBS+=$(MINGW_SHARED_LIBSTDCPP)
.ENDIF
CONFIGURE_DIR=
CONFIGURE_ACTION=.$/configure
CONFIGURE_FLAGS=--enable-ipv6=no --without-python --without-zlib --enable-static=no --without-debug --build=i586-pc-mingw32 --host=i586-pc-mingw32 lt_cv_cc_dll_switch="-shared" CC="$(xml2_CC)" LDFLAGS="-no-undefined -Wl,--enable-runtime-pseudo-reloc-v2 -L$(ILIB:s/;/ -L/)" LIBS="$(xml2_LIBS)" OBJDUMP=objdump
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
xml2_CFLAGS+=$(ARCH_FLAGS) $(C_RESTRICTIONFLAGS)
.ENDIF			# "$(COMNAME)"=="sunpro5"
xml2_LDFLAGS+=-L$(SYSBASE)$/usr$/lib
.ENDIF			# "$(SYSBASE)"!=""

CONFIGURE_DIR=
.IF "$(OS)"=="OS2"
CONFIGURE_ACTION=sh .$/configure
CONFIGURE_FLAGS=--enable-ipv6=no --without-python --without-zlib --enable-static=yes --with-sax1=yes ADDCFLAGS="$(xml2_CFLAGS)" CFLAGS="$(EXTRA_CFLAGS)" LDFLAGS="$(xml2_LDFLAGS) $(EXTRA_LINKFLAGS)"
.ELSE
CONFIGURE_ACTION=.$/configure
CONFIGURE_FLAGS=--enable-ipv6=no --without-python --without-zlib --enable-static=no --with-sax1=yes ADDCFLAGS="$(xml2_CFLAGS) $(EXTRA_CFLAGS)" LDFLAGS="$(xml2_LDFLAGS) $(EXTRA_LINKFLAGS)"
.ENDIF
BUILD_ACTION=$(GNUMAKE)
BUILD_FLAGS+= -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)
.ENDIF


OUTDIR2INC=include$/libxml 

.IF "$(OS)"=="MACOSX"
EXTRPATH=URELIB
OUT2LIB+=.libs$/libxml2.*.dylib
OUT2BIN+=.libs$/xmllint
OUT2BIN+=xml2-config
.ELIF "$(OS)"=="WNT"
.IF "$(COM)"=="GCC"
OUT2LIB+=.libs$/libxml2*.a
OUT2BIN+=.libs$/libxml2*.dll
OUT2BIN+=.libs$/xmllint.exe
OUT2BIN+=xml2-config
.ELSE
OUT2LIB+=win32$/bin.msvc$/*.lib
OUT2BIN+=win32$/bin.msvc$/*.dll
OUT2BIN+=win32$/bin.msvc$/xmllint.exe
.ENDIF
.ELSE
OUT2LIB+=.libs$/libxml2.so*
OUT2BIN+=.libs$/xmllint
OUT2BIN+=xml2-config
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

