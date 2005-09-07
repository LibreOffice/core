#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 19:37:59 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

PRJ=.

PRJNAME=xmlsec1
TARGET=so_xmlsec1

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
CONFIGURE_DIR=win32
CONFIGURE_ACTION=cscript configure.js
CONFIGURE_FLAGS=crypto=$(CRYPTOLIB) xslt=no iconv=no static=no include=$(BASEINC) lib=$(BASELIB)
BUILD_ACTION=nmake
BUILD_DIR=$(CONFIGURE_DIR)
.ELSE
.IF "$(GUI)"=="UNX"
.IF "$(OS)$(COM)"=="LINUXGCC"
LDFLAGS:=-Wl,-rpath,'$$$$ORIGIN'
.ENDIF			# "$(OS)$(COM)"=="LINUXGCC"
.IF "$(OS)$(COM)"=="SOLARISC52"
LDFLAGS:=-Wl,-R'$$$$ORIGIN'
.ENDIF			# "$(OS)$(COM)"=="SOLARISC52"
.EXPORT: LDFLAGS
.ENDIF
CONFIGURE_DIR=
CONFIGURE_ACTION=chmod 777 libxml2-config && .$/configure
CONFIGURE_FLAGS=--with-libxslt=no --with-openssl=no --with-gnutls=no
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
OUT2LIB+=win32$/binaries$/*.lib
OUT2BIN+=win32$/binaries$/*.dll
.ELSE
OUT2LIB+=src$/.libs$/libxmlsec1.so* src$/nss$/.libs$/libxmlsec1-nss.so*
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk


