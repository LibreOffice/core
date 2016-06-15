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

PRJNAME=libxslt
TARGET=so_libxslt

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(SYSTEM_LIBXSLT)" == "YES"
all:
    @echo "An already available installation of libxslt should exist on your system."
    @echo "Therefore the version provided here does not need to be built in addition."
.ENDIF

# --- Files --------------------------------------------------------

.IF "$(L10N_framework)"==""

.INCLUDE :	libxsltversion.mk

LIBXSLTVERSION=$(LIBXSLT_MAJOR).$(LIBXSLT_MINOR).$(LIBXSLT_MICRO)

TARFILE_NAME=$(PRJNAME)-$(LIBXSLTVERSION)
TARFILE_MD5=9667bf6f9310b957254fdcf6596600b7

# libxslt-internal-symbols: #i112480#: Solaris ld requires symbols to be defined
PATCH_FILES=libxslt-configure.patch \
            libxslt-win_manifest.patch \
            libxslt-CVE-2015-7995.patch



# This is only for UNX environment now
.IF "$(OS)"=="WNT"
.IF "$(COM)"=="GCC"
xslt_CC=$(CC) -mthreads
.IF "$(MINGW_SHARED_GCCLIB)"=="YES"
xslt_CC+=-shared-libgcc
.ENDIF
xslt_LIBS=
.IF "$(MINGW_SHARED_GXXLIB)"=="YES"
xslt_LIBS+=$(MINGW_SHARED_LIBSTDCPP)
.ENDIF
CONFIGURE_DIR=
CONFIGURE_ACTION=.$/configure
CONFIGURE_FLAGS=--without-crypto --without-python --enable-static=no --build=i586-pc-mingw32 --host=i586-pc-mingw32 CC="$(xslt_CC)" CFLAGS="$(xslt_CFLAGS)" LDFLAGS="-no-undefined -Wl,--enable-runtime-pseudo-reloc-v2 -L$(ILIB:s/;/ -L/)" LIBS="$(xslt_LIBS)"  LIBXML2LIB=$(LIBXML2LIB) OBJDUMP=objdump
BUILD_ACTION=chmod 777 xslt-config && $(GNUMAKE)
BUILD_FLAGS+= -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)
.IF "$(GUI)$(COM)"=="WNTGCC"
.EXPORT : PWD
.ENDIF
.ELSE
CONFIGURE_DIR=win32
CONFIGURE_ACTION=cscript configure.js
#CONFIGURE_FLAGS=iconv=no sax1=yes
.IF "$(debug)"!=""
CONFIGURE_FLAGS+=debug=yes
.ENDIF
BUILD_ACTION=nmake
BUILD_DIR=$(CONFIGURE_DIR)
.ENDIF
.ELSE

.IF "$(OS)$(COM)"=="LINUXGCC" || "$(OS)$(COM)"=="FREEBSDGCC"
LDFLAGS:=-Wl,-rpath,'$$$$ORIGIN:$$$$ORIGIN/../ure-link/lib' -Wl,-noinhibit-exec
.ENDIF                  # "$(OS)$(COM)"=="LINUXGCC"
.IF "$(OS)$(COM)"=="SOLARISC52"
LDFLAGS:=-Wl,-R'$$$$ORIGIN:$$$$ORIGIN/../ure-link/lib'
.ENDIF                  # "$(OS)$(COM)"=="SOLARISC52"

.IF "$(SYSBASE)"!=""
CPPFLAGS+:=-I$(SOLARINCDIR)$/external -I$(SYSBASE)$/usr$/include $(EXTRA_CFLAGS)
.IF "$(OS)"=="SOLARIS" || "$(OS)"=="LINUX"
LDFLAGS+:=-L$(SOLARLIBDIR) -L$(SYSBASE)$/lib -L$(SYSBASE)$/usr$/lib -lpthread -ldl
.ENDIF
.ENDIF			# "$(SYSBASE)"!=""

.EXPORT: CPPFLAGS
.EXPORT: LDFLAGS
.EXPORT: LIBXML2LIB

.IF "$(COMNAME)"=="sunpro5"
CPPFLAGS+:=$(ARCH_FLAGS) -xc99=none
.ENDIF                  # "$(COMNAME)"=="sunpro5"
CONFIGURE_DIR=
CONFIGURE_ACTION=.$/configure
CONFIGURE_FLAGS=--enable-ipv6=no --without-crypto --without-python --enable-static=no --with-sax1=yes ac_cv_func_clock_gettime=false
BUILD_ACTION=chmod 777 xslt-config && $(GNUMAKE)
BUILD_FLAGS+= -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)
.ENDIF

OUT2INC=libxslt$/*.h

.IF "$(OS)"=="MACOSX"
OUT2LIB+=libxslt$/.libs$/libxslt.*.dylib
OUT2LIB+=libexslt$/.libs$/libexslt.*.dylib
OUT2BIN+=xsltproc$/.libs$/xsltproc
OUT2BIN+=xslt-config
.ELIF "$(OS)"=="WNT"
.IF "$(COM)"=="GCC"
OUT2LIB+=libxslt$/.libs$/*.a
OUT2LIB+=libexslt$/.libs$/*.a
OUT2BIN+=libxslt$/.libs$/*.dll
OUT2BIN+=libexslt$/.libs$/*.dll
OUT2BIN+=xsltproc$/.libs$/*.exe*
OUT2BIN+=xslt-config
.ELSE
OUT2LIB+=win32$/bin.msvc$/*.lib
OUT2BIN+=win32$/bin.msvc$/*.dll
OUT2BIN+=win32$/bin.msvc$/*.exe
.ENDIF
.ELSE
OUT2LIB+=libxslt$/.libs$/libxslt.so*
OUT2LIB+=libexslt$/.libs$/libexslt.so*
OUT2BIN+=xsltproc$/.libs$/xsltproc
OUT2BIN+=xslt-config
.ENDIF

# --- Targets ------------------------------------------------------
.ENDIF 			# L10N_framework
.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

