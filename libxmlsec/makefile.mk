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

PRJNAME=xmlsec1
TARGET=so_xmlsec1
EXTERNAL_WARNINGS_NOT_ERRORS := TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(OS)" == "ANDROID" || "$(OS)" == "IOS"
@all:
	@echo "FIXME: XMLSec module not buildable yet on this platform"
.ENDIF

# --- Files --------------------------------------------------------

XMLSEC1VERSION=1.2.14

TARFILE_NAME=$(PRJNAME)-$(XMLSEC1VERSION)
TARFILE_MD5=1f24ab1d39f4a51faf22244c94a6203f

#xmlsec1-configure.patch: Set up the build. Straightforward configuration
#xmlsec1-configure-libxml-libxslt.patch: empty "$with_libxml" prepends /bin :-(
#xmlsec1-olderlibxml2.patch: Allow build against older libxml2, for macosx
#xmlsec1-nssdisablecallbacks.patch: Disable use of smime3 so don't need to package it
#xmlsec1-customkeymanage.patch: Could we do this alternatively outside xmlsec
#xmlsec1-nssmangleciphers.patch: Dubious, do we still need this ?
#xmlsec1-noverify.patch: As per readme.txt. 
#xmlsec1-mingw32.patch: Mingw32 support. 
#xmlsec1-mingw-customkeymanage-addmscrypto.patch: builds the custom keymanager on mingw
#xmlsec1-vc.path: support for Visual C++ 10
PATCH_FILES=\
   xmlsec1-configure.patch \
   xmlsec1-configure-libxml-libxslt.patch \
   xmlsec1-olderlibxml2.patch \
   xmlsec1-nssdisablecallbacks.patch \
   xmlsec1-customkeymanage.patch \
   xmlsec1-nssmangleciphers.patch \
   xmlsec1-noverify.patch \
   xmlsec1-mingw32.patch \
   xmlsec1-mingw-keymgr-mscrypto.patch \
   xmlsec1-vc10.patch \
   xmlsec1-1.2.14_fix_extern_c.patch \
   xmlsec1-android.patch

ADDITIONAL_FILES= \
    include$/xmlsec$/mscrypto$/akmngr.h \
    src$/mscrypto$/akmngr.c \
    include$/xmlsec$/nss$/akmngr.h \
    include$/xmlsec$/nss$/ciphers.h \
    include$/xmlsec$/nss$/tokens.h \
    src$/nss$/akmngr.c \
    src$/nss$/keywrapers.c \
    src$/nss$/tokens.c

.IF "$(GUI)"=="WNT"
CRYPTOLIB=mscrypto
.ELSE
CRYPTOLIB=nss
.ENDIF

.IF "$(OS)"=="WNT"
.IF "$(COM)"=="GCC"
xmlsec_CC=$(CC) -mthreads
.IF "$(MINGW_SHARED_GCCLIB)"=="YES"
xmlsec_CC+=-shared-libgcc
.ENDIF
xmlsec_LIBS=
.IF "$(MINGW_SHARED_GXXLIB)"=="YES"
xmlsec_LIBS+=$(MINGW_SHARED_LIBSTDCPP)
.ENDIF
CONF_ILIB=
.IF "$(ILIB)" != ""
CONF_ILIB=-L$(ILIB:s/;/ -L/)
.ENDIF

CONFIGURE_DIR=
CONFIGURE_ACTION=autoreconf ; .$/configure

.IF "$(CROSS_COMPILING)"=="YES"
BUILD_AND_HOST=--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM) MINGW_SYSROOT=$(MINGW_SYSROOT)
.ELSE
BUILD_AND_HOST=--build=i586-pc-mingw32 --host=i586-pc-mingw32 --with-mozilla_ver=1.7.5 --enable-mscrypto
.ENDIF

# Note that this is obsolete crack for building *locally* on Windows with MinGW,
# something we don't see the point in here in LibreOffice

CONFIGURE_FLAGS=--with-libxslt=no --with-openssl=no --with-gnutls=no --disable-crypto-dl $(BUILD_AND_HOST) CC="$(xmlsec_CC)" LDFLAGS="-Wl,--no-undefined $(CONF_ILIB)" LIBS="$(xmlsec_LIBS)" LIBXML2LIB="$(LIBXML2LIB)" ZLIB3RDLIB=$(ZLIB3RDLIB) OBJDUMP="$(WRAPCMD) objdump"

.IF "$(SYSTEM_NSS)" != "YES"
CONFIGURE_FLAGS+=--enable-pkgconfig=no
.ENDIF
BUILD_ACTION=$(GNUMAKE) -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)
.ELSE
CONFIGURE_DIR=win32
CONFIGURE_ACTION=cscript configure.js
.IF "$(product)"!="full" && "$(CCNUMVER)" >= "001399999999"
CONFIGURE_FLAGS=crypto=$(CRYPTOLIB) debug=yes xslt=no iconv=no static=no include=$(BASEINC) lib=$(BASELIB)
.ELSE
CONFIGURE_FLAGS=crypto=$(CRYPTOLIB) xslt=no iconv=no static=no include=$(BASEINC) lib=$(BASELIB)
.ENDIF
BUILD_ACTION=nmake
BUILD_DIR=$(CONFIGURE_DIR)
.ENDIF
.ELSE
.IF "$(GUI)"=="UNX"

.IF "$(COM)"=="C52" && "$(CPU)"=="U"
xmlsec_CFLAGS+=-m64
.ENDIF

.IF "$(SYSBASE)"!=""
xmlsec_CFLAGS+=-I$(SYSBASE)$/usr$/include 
.IF "$(COMNAME)"=="sunpro5"
xmlsec_CFLAGS+=$(C_RESTRICTIONFLAGS)
.ENDIF			# "$(COMNAME)"=="sunpro5"
.IF "$(EXTRA_CFLAGS)"!=""
xmlsec_CFLAGS+=$(EXTRA_CFLAGS)
xmlsec_CPPFLAGS+=$(EXTRA_CFLAGS)
.ENDIF # "$(EXTRA_CFLAGS)"!=""
xmlsec_LDFLAGS+=-L$(SYSBASE)$/usr$/lib
.ELIF "$(OS)"=="MACOSX" # "$(SYSBASE)"!=""
xmlsec_CPPFLAGS+=$(EXTRA_CDEFS)
.ENDIF

.IF "$(OS)$(COM)"=="LINUXGCC" || "$(OS)$(COM)"=="FREEBSDGCC"
xmlsec_LDFLAGS+=-Wl,-rpath,'$$$$ORIGIN:$$$$ORIGIN/../ure-link/lib'
.ENDIF			# "$(OS)$(COM)"=="LINUXGCC"
.IF "$(OS)$(COM)"=="SOLARISC52"
xmlsec_LDFLAGS+=-Wl,-R'$$$$ORIGIN:$$$$ORIGIN/../ure-link/lib'
.ENDIF			# "$(OS)$(COM)"=="SOLARISC52"

LDFLAGS:=$(xmlsec_LDFLAGS)
.EXPORT: LDFLAGS

.ENDIF
CONFIGURE_DIR=
CONFIGURE_ACTION=autoreconf ; .$/configure ADDCFLAGS="$(xmlsec_CFLAGS)" CPPFLAGS="$(xmlsec_CPPFLAGS)"
CONFIGURE_FLAGS=--with-pic --disable-shared --disable-crypto-dl --with-libxslt=no --with-gnutls=no LIBXML2LIB="$(LIBXML2LIB)"

.IF "$(CROSS_COMPILING)"=="YES"
CONFIGURE_FLAGS+= --build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)
.ENDIF

.IF "$(OS)" == "ANDROID"
CONFIGURE_FLAGS+=--with-openssl=$(SOLARVER)/$(INPATH)
.ELSE
CONFIGURE_FLAGS+=--with-openssl=no
.ENDIF

.IF "$(OS)" == "MACOSX"
CONFIGURE_FLAGS += \
    --prefix=/@.__________________________________________________$(EXTRPATH)
.END

# system-mozilla needs pkgconfig to get the information about nss
# FIXME: This also will enable pkg-config usage for libxml2. It *seems*
# that the internal headers still are used when they are there but....
# (and that pkg-config is allowed to fail...)
# I have no real good idea how to get mozilla (nss) pkg-config'ed and libxml2
# not... We need mozilla-nss pkg-config'ed since we can *not* just use
# --with-nss or parse -pkg-config --libs / cflags mozilla-nss since
# the lib may a) be in /usr/lib (Debian) and be not in $with_nss/include
# $with_nss/lib.
.IF "$(SYSTEM_NSS)" != "YES"
CONFIGURE_FLAGS+=--enable-pkgconfig=no
.ENDIF
BUILD_ACTION=$(GNUMAKE) -j$(EXTMAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)
.ENDIF


OUTDIR2INC=include$/xmlsec 

.IF "$(OS)"=="WNT"
.IF "$(COM)"=="GCC"
OUT2LIB+=src$/.libs$/libxmlsec1.dll.a src$/nss$/.libs$/libxmlsec1-nss.dll.a
OUT2BIN+=src$/.libs$/libxmlsec1.dll src$/nss$/.libs$/libxmlsec1-nss.dll
.IF "$(CROSS_COMPILING)" != "YES"
OUT2LIB+=src$/mscrypto$/.libs$/libxmlsec1-mscrypto.dll.a
OUT2BIN+=src$/mscrypto$/.libs$/libxmlsec1-mscrypto.dll
.ENDIF
.ELSE
OUT2LIB+=win32$/binaries$/*.lib
OUT2BIN+=win32$/binaries$/*.dll
.ENDIF
.ELIF "$(OS)" == "ANDROID"
OUT2LIB+=src$/.libs$/libxmlsec1.a src$/openssl/.libs$/libxmlsec1-openssl.a
.ELSE
OUT2LIB+=src$/.libs$/libxmlsec1.a src$/nss$/.libs$/libxmlsec1-nss.a
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk


