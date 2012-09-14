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
PRJ=.

PRJNAME=so_curl
TARGET=so_curl

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(SYSTEM_CURL)" == "YES"
all:
    @echo "An already available installation of curl should exist on your system."
    @echo "Therefore the version provided here does not need to be built in addition."
.ENDIF

# --- Files --------------------------------------------------------

TARFILE_NAME=curl-7.26.0
TARFILE_MD5=3fa4d5236f2a36ca5c3af6715e837691
PATCH_FILES=\
    curl-7.26.0.patch \
    curl-7.26.0_nspr.patch \
    curl-aix.patch

.IF "$(GUI)"=="WNT"
    PATCH_FILES+=curl-7.26.0_win.patch
    .IF "$(COM)"=="GCC"
        PATCH_FILES+=curl-7.26.0_mingw.patch
    .ENDIF
.ENDIF

.IF "$(OS)" == "ANDROID"
    PATCH_FILES+=curl-android.patch
.ENDIF

#CONVERTFILES= \
    lib$/Makefile.vc6

#ADDITIONAL_FILES= lib$/config-os2.h lib$/Makefile.os2

.IF "$(GUI)"=="UNX"

.IF "$(SYSBASE)"!=""
curl_CFLAGS+=-I$(SYSBASE)$/usr$/include
curl_LDFLAGS+=-L$(SYSBASE)$/usr$/lib
.ENDIF			# "$(SYSBASE)"!=""

.IF "$(OS)$(CPU)"=="SOLARISU"
curl_CFLAGS+:=$(ARCH_FLAGS)
curl_LDFLAGS+:=$(ARCH_FLAGS)
.ENDIF

.IF "$(OS)"=="AIX"
curl_LDFLAGS+:=$(LINKFLAGS) $(LINKFLAGSRUNPATH_OOO)
.ENDIF

CONFIGURE_DIR=.$/
#relative to CONFIGURE_DIR
CONFIGURE_ACTION=.$/configure
.IF "$(DISABLE_DYNLOADING)" == "TRUE"
CONFIGURE_FLAGS=--disable-shared
.ELSE
CONFIGURE_FLAGS=--disable-static
.ENDIF
CONFIGURE_FLAGS+= --with-nss --without-ssl --without-libidn --enable-ftp --enable-ipv6 --enable-http --disable-gopher --disable-file --disable-ldap --disable-telnet --disable-dict --without-libssh2 CPPFLAGS="$(curl_CFLAGS)"  LDFLAGS="$(curl_LDFLAGS)"
.IF "$(debug)" != ""
CONFIGURE_FLAGS+=--enable-debug
.ENDIF

.IF "$(OS)" == "MACOSX"
CONFIGURE_FLAGS += \
    --prefix=/@.__________________________________________________$(EXTRPATH)
.END

.IF "$(CROSS_COMPILING)"=="YES"
CONFIGURE_FLAGS+=--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)
.ENDIF

BUILD_DIR=$(CONFIGURE_DIR)$/lib
BUILD_ACTION=$(GNUMAKE)
BUILD_FLAGS+= -j$(EXTMAXPROCESS)

.IF "$(DISABLE_DYNLOADING)" == "TRUE"
OUT2LIB=$(BUILD_DIR)$/.libs$/libcurl.a
.ELIF "$(OS)"=="ANDROID"
OUT2LIB=$(BUILD_DIR)$/.libs$/libcurl.so
.ELSE
OUT2LIB=$(BUILD_DIR)$/.libs$/libcurl$(DLLPOST).?
.ENDIF
.ENDIF			# "$(GUI)"=="UNX"


.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
curl_CC=$(CC) -mthreads
.IF "$(MINGW_SHARED_GCCLIB)"=="YES"
curl_CC+=-shared-libgcc
.ENDIF
curl_LIBS=-lws2_32 -lwinmm
.IF "$(MINGW_SHARED_GXXLIB)"=="YES"
curl_LIBS+=$(MINGW_SHARED_LIBSTDCPP)
.ENDIF
CONFIGURE_DIR=.$/
#relative to CONFIGURE_DIR
CONFIGURE_ACTION=.$/configure
CONFIGURE_FLAGS= --with-nss --without-ssl --enable-ftp --enable-ipv6 --disable-http --disable-gopher --disable-file --disable-ldap --disable-telnet --disable-dict --build=i586-pc-mingw32 --host=i586-pc-mingw32 CC="$(curl_CC)" CPPFLAGS="$(INCLUDE)" OBJDUMP="objdump" LDFLAGS="-L$(ILIB:s/;/ -L/)" LIBS="$(curl_LIBS)"
.IF "$(debug)" != ""
CONFIGURE_FLAGS+=--enable-debug
.ENDIF
BUILD_DIR=$(CONFIGURE_DIR)$/lib
BUILD_ACTION=make
OUT2BIN=$(BUILD_DIR)$/.libs$/libcurl*.dll
OUT2LIB=$(BUILD_DIR)$/.libs$/libcurl*.a
.ELSE

.IF "$(CCNUMVER)" > "001399999999"
EXCFLAGS="/EHa /Zc:wchar_t- /D "_CRT_SECURE_NO_DEPRECATE""
.ELSE
EXCFLAGS="/EHsc /YX"
.ENDIF

BUILD_DIR=.$/lib

.IF "$(CPUNAME)" == "INTEL"
MACHINE=X86
.ELSE
MACHINE=X64
.ENDIF

.IF "$(debug)"==""
BUILD_ACTION=nmake -f Makefile.vc9 cfg=release-dll EXCFLAGS=$(EXCFLAGS) MACHINE=$(MACHINE)
.ELSE
BUILD_ACTION=nmake -f Makefile.vc9 cfg=debug-dll EXCFLAGS=$(EXCFLAGS) MACHINE=$(MACHINE)
.ENDIF

OUT2BIN=$(BUILD_DIR)$/libcurl.dll
OUT2LIB=$(BUILD_DIR)$/libcurl.lib

.ENDIF
.ENDIF			# "$(GUI)"=="WNT"

OUT2INC= \
    include$/curl$/easy.h  			\
    include$/curl$/multi.h  		\
    include$/curl$/curl.h  			\
    include$/curl$/curlver.h  		\
    include$/curl$/typecheck-gcc.h	\
    include$/curl$/stdcheaders.h  	\
    include$/curl$/mprintf.h	    \
    include$/curl$/curlbuild.h		\
    include$/curl$/curlrules.h

.IF "$(GUI)"=="UNX" || "$(COM)"=="GCC"
OUT2INC+=libcurl.pc
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk
