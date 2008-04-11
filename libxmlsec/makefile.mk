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
# $Revision: 1.17 $
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

.IF "$(WITH_MOZILLA)" == "NO"
@all:
    @echo "Mozilla disabled -> no nss -> no libxmlsec...."
.ENDIF

# --- Files --------------------------------------------------------

XMLSEC1VERSION=1.2.6

TARFILE_NAME=$(PRJNAME)-$(XMLSEC1VERSION)
PATCH_FILE_NAME=$(TARFILE_NAME).patch

ADDITIONAL_FILES= \
    include$/xmlsec$/nss$/akmngr.h \
    include$/xmlsec$/nss$/ciphers.h \
    include$/xmlsec$/nss$/tokens.h \
    include$/xmlsec$/mscrypto$/akmngr.h \
    src$/nss$/akmngr.c \
    src$/mscrypto$/akmngr.c \
    src$/nss$/keytrans.c \
    src$/nss$/keywrapers.c \
    src$/nss$/tokens.c \
    xmlsec-mscrypto.pc.in \
    include$/xmlsec$/mscrypto$/Makefile.in \
    src$/mscrypto$/Makefile.in \
    libxml2-config

.IF "$(GUI)"=="WNT"
CRYPTOLIB=mscrypto
#CRYPTOLIB=nss
#BASEINC=$(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/mozilla;$(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/mozilla$/nspr;$(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/mozilla$/nss;$(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/external
#BASELIB=$(SOLARVERSION)$/$(INPATH)$/lib$(UPDMINOREXT)
.ELSE
CRYPTOLIB=nss
.ENDIF

.IF "$(OS)"=="WNT"
.IF "$(COM)"=="GCC"
CONFIGURE_DIR=
CONFIGURE_ACTION=chmod 777 libxml2-config && .$/configure
CONFIGURE_FLAGS=--with-libxslt=no --with-openssl=no --with-gnutls=no --with-mozilla_ver=1.7.5 --with-mscrypto --build=i586-pc-mingw32 --host=i586-pc-mingw32 CFLAGS="-D_MT" LDFLAGS="-no-undefined -L$(ILIB:s/;/ -L/)" LIBS="-lmingwthrd" LIBXML2LIB="$(LIBXML2LIB)" ZLIB3RDLIB=$(ZLIB3RDLIB) OBJDUMP="$(WRAPCMD) objdump"
.IF "$(SYSTEM_MOZILLA)" != "YES"
CONFIGURE_FLAGS+=--enable-pkgconfig=no
.ENDIF
BUILD_ACTION=$(GNUMAKE)
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

.IF "$(SYSBASE)"!=""
xmlsec_CFLAGS+=-I$(SYSBASE)$/usr$/include 
.IF "$(COMNAME)"=="sunpro5"
xmlsec_CFLAGS+=$(C_RESTRICTIONFLAGS)
.ENDIF			# "$(COMNAME)"=="sunpro5"
.IF "$(EXTRA_CFLAGS)"!=""
xmlsec_CFLAGS+=$(EXTRA_CFLAGS)
.ENDIF # "$(EXTRA_CFLAGS)"!=""
xmlsec_LDFLAGS+=-L$(SYSBASE)$/usr$/lib
.ENDIF			# "$(SYSBASE)"!=""

.IF "$(OS)$(COM)"=="LINUXGCC" || "$(OS)$(COM)"=="FREEBSDGCC"
xmlsec_LDFLAGS+=-Wl,-rpath,'$$$$ORIGIN:$$$$ORIGIN/../ure-link/lib'
.ENDIF			# "$(OS)$(COM)"=="LINUXGCC"
.IF "$(OS)$(COM)"=="SOLARISC52"
xmlsec_LDFLAGS+=-Wl,-R'$$$$ORIGIN:$$$$ORIGIN/../ure-link/lib'
.ENDIF			# "$(OS)$(COM)"=="SOLARISC52"

.IF "$(OS)$(COM)"=="LINUXGCC"
xmlsec_LDFLAGS+=-Wl,-z,noexecstack
.ENDIF

LDFLAGS:=$(xmlsec_LDFLAGS)
.EXPORT: LDFLAGS

#.IF "$(OS)$(COM)"=="LINUXGCC"
#LDFLAGS:=-Wl,-rpath,'$$$$ORIGIN'
#.ENDIF			# "$(OS)$(COM)"=="LINUXGCC"
#.IF "$(OS)$(COM)"=="SOLARISC52"
#LDFLAGS:=-Wl,-R'$$$$ORIGIN'
#.ENDIF			# "$(OS)$(COM)"=="SOLARISC52"
#.EXPORT: LDFLAGS
.ENDIF
CONFIGURE_DIR=
#CONFIGURE_ACTION=chmod 777 libxml2-config && .$/configure CFLAGS="$(xmlsec_CFLAGS)" CPPFLAGS="$(xmlsec_CPPFLAGS)" LDFLAGS="$(xmlsec_LDFLAGS)"
CONFIGURE_ACTION=chmod 777 libxml2-config && .$/configure ADDCFLAGS="$(xmlsec_CFLAGS)" CPPFLAGS="$(xmlsec_CPPFLAGS)"
CONFIGURE_FLAGS=--with-libxslt=no --with-openssl=no --with-gnutls=no LIBXML2LIB="$(LIBXML2LIB)" ZLIB3RDLIB=$(ZLIB3RDLIB)
# system-mozilla needs pkgconfig to get the information about nss
# FIXME: This also will enable pkg-config usage for libxml2. It *seems*
# that the internal headers still are used when they are there but....
# (and that pkg-config is allowed to fail...)
# I have no real good idea how to get mozilla (nss) pkg-config'ed and libxml2
# not... We need mozilla-nss pkg-config'ed since we can *not* just use
# --with-nss or parse -pkg-config --libs / cflags mozilla-nss since
# the lib may a) be in /usr/lib (Debian) and be not in $with_nss/include
# $with_nss/lib.
.IF "$(SYSTEM_MOZILLA)" != "YES"
CONFIGURE_FLAGS+=--enable-pkgconfig=no
.ENDIF
BUILD_ACTION=$(GNUMAKE)
BUILD_DIR=$(CONFIGURE_DIR)
.ENDIF


OUTDIR2INC=include$/xmlsec 

.IF "$(OS)"=="MACOSX"
OUT2LIB+=src$/.libs$/libxmlsec1.*dylib src$/nss$/.libs$/libxmlsec1-nss.*dylib 
.ELIF "$(OS)"=="WNT"
.IF "$(COM)"=="GCC"
OUT2BIN+=src$/.libs$/libxmlsec1-1.dll src$/nss$/.libs$/libxmlsec1-nss-1.dll src$/mscrypto$/.libs$/libxmlsec1-mscrypto-1.dll
.ELSE
OUT2LIB+=win32$/binaries$/*.lib
OUT2BIN+=win32$/binaries$/*.dll
.ENDIF
.ELSE
OUT2LIB+=src$/.libs$/libxmlsec1.so* src$/nss$/.libs$/libxmlsec1-nss.so*
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk


