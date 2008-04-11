#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.24 $
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

TARFILE_NAME=curl-7.12.2
PATCH_FILE_NAME=curl-7.12.2.patch
CONVERTFILES= \
    lib$/Makefile.vc6

ADDITIONAL_FILES= lib$/config-os2.h lib$/Makefile.os2

.IF "$(GUI)"=="UNX"

.IF "$(SYSBASE)"!=""
curl_CFLAGS+=-I$(SYSBASE)$/usr$/include
curl_LDFLAGS+=-L$(SYSBASE)$/usr$/lib
.ENDIF			# "$(SYSBASE)"!=""

.IF "$(OS)$(COM)$(CPU)"=="LINUXGCCI"
curl_LDFLAGS+=-Wl,-z,noexecstack
.ENDIF

CONFIGURE_DIR=.$/
#relative to CONFIGURE_DIR
CONFIGURE_ACTION=.$/configure
CONFIGURE_FLAGS= --without-ssl --without-libidn --enable-ftp --enable-ipv6 --enable-http --disable-gopher --disable-file --disable-ldap --disable-telnet --disable-dict --disable-static CPPFLAGS="$(curl_CFLAGS)"  LDFLAGS="$(curl_LDFLAGS)"

BUILD_DIR=$(CONFIGURE_DIR)$/lib
.IF "$(OS)"=="IRIX"
BUILD_ACTION=gmake
.ELSE
BUILD_ACTION=$(GNUMAKE)
.ENDIF
BUILD_FLAGS+= -j$(EXTMAXPROCESS)

OUT2LIB=$(BUILD_DIR)$/.libs$/libcurl*$(DLLPOST)*
.ENDIF			# "$(GUI)"=="UNX"


.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
CONFIGURE_DIR=.$/
#relative to CONFIGURE_DIR
CONFIGURE_ACTION=.$/configure
CONFIGURE_FLAGS= --without-ssl --enable-ftp --enable-ipv6 --disable-http --disable-gopher --disable-file --disable-ldap --disable-telnet --disable-dict --build=i586-pc-mingw32 --host=i586-pc-mingw32 OBJDUMP="$(WRAPCMD) objdump" CFLAGS=-D_MT LDFLAGS="-L$(ILIB:s/;/ -L/)" LIBS="-lws2_32 -lwinmm -lmingwthrd"
BUILD_DIR=$(CONFIGURE_DIR)$/lib
BUILD_ACTION=make
OUT2BIN=$(BUILD_DIR)$/.libs$/libcurl*.dll
OUT2LIB=$(BUILD_DIR)$/.libs$/libcurl*.a
.ELSE
# make use of stlport headerfiles
EXT_USE_STLPORT=TRUE

.IF "$(CCNUMVER)" > "001399999999"
EXCFLAGS="/EHa /Zc:wchar_t- /D "_CRT_SECURE_NO_DEPRECATE""
.ELSE
EXCFLAGS="/EHsc /YX"
.ENDIF

BUILD_DIR=.$/lib
.IF "$(debug)"==""
BUILD_ACTION=nmake -f Makefile.vc6 cfg=release-dll EXCFLAGS=$(EXCFLAGS)
.ELSE
BUILD_ACTION=nmake -f Makefile.vc6 cfg=debug-dll EXCFLAGS=$(EXCFLAGS)
.ENDIF

OUT2BIN=$(BUILD_DIR)$/libcurl.dll
OUT2LIB=$(BUILD_DIR)$/libcurl.lib

.ENDIF
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="OS2"
# make use of stlport headerfiles
EXT_USE_STLPORT=TRUE

BUILD_DIR=.$/lib
.IF "$(debug)"==""
BUILD_ACTION=make -f Makefile.os2
.ELSE
BUILD_ACTION=make -f Makefile.os2
.ENDIF

OUT2BIN=$(BUILD_DIR)$/libcurl.dll
OUT2LIB=$(BUILD_DIR)$/libcurl.lib

.ENDIF			# "$(GUI)"=="OS2"

OUT2INC= \
    include$/curl$/easy.h  			\
    include$/curl$/multi.h  		\
    include$/curl$/curl.h  			\
    include$/curl$/curlver.h  		\
    include$/curl$/types.h  		\
    include$/curl$/stdcheaders.h  	\
    include$/curl$/mprintf.h

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk
