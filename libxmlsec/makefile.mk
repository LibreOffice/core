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

PRJNAME=xmlsec1
TARGET=so_xmlsec1
EXTERNAL_WARNINGS_NOT_ERRORS := TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(ENABLE_NSS_MODULE)" == "NO"
@all:
    @echo "No nss -> no libxmlsec...."
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
#xmlsec1-mingw-customkeymanage-addmscrypto.patch builds the custom keymanager on mingw
PATCH_FILES=\
   xmlsec1-configure.patch \
   xmlsec1-configure-libxml-libxslt.patch \
   xmlsec1-olderlibxml2.patch \
   xmlsec1-nssdisablecallbacks.patch \
   xmlsec1-customkeymanage.patch \
   xmlsec1-nssmangleciphers.patch \
   xmlsec1-noverify.patch \
   xmlsec1-mingw32.patch \
   xmlsec1-mingw-keymgr-mscrypto.patch


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
#CRYPTOLIB=nss
#BASEINC=$(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/mozilla;$(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/mozilla$/nspr;$(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/mozilla$/nss;$(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/external
#BASELIB=$(SOLARVERSION)$/$(INPATH)$/lib$(UPDMINOREXT)
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
CONFIGURE_DIR=
CONFIGURE_ACTION=.$/configure
CONFIGURE_FLAGS=--with-libxslt=no --with-openssl=no --with-gnutls=no --enable-mscrypto --disable-crypto-dl --build=i586-pc-mingw32 --host=i586-pc-mingw32 CC="$(xmlsec_CC)" LDFLAGS="-no-undefined -L$(ILIB:s/;/ -L/)" LIBS="$(xmlsec_LIBS)" LIBXML2LIB=$(LIBXML2LIB) ZLIB3RDLIB=$(ZLIB3RDLIB) OBJDUMP="$(WRAPCMD) objdump"

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
CONFIGURE_ACTION=.$/configure ADDCFLAGS="$(xmlsec_CFLAGS)" CPPFLAGS="$(xmlsec_CPPFLAGS)"
CONFIGURE_FLAGS=--with-pic --disable-shared --disable-crypto-dl --with-libxslt=no --with-openssl=no --with-gnutls=no LIBXML2LIB="$(LIBXML2LIB)"
# system-nss needs pkgconfig to get the information about nss
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
OUT2LIB+=src$/.libs$/libxmlsec1.dll.a src$/nss$/.libs$/libxmlsec1-nss.dll.a src$/mscrypto$/.libs$/libxmlsec1-mscrypto.dll.a
OUT2BIN+=src$/.libs$/libxmlsec1.dll src$/nss$/.libs$/libxmlsec1-nss.dll src$/mscrypto$/.libs$/libxmlsec1-mscrypto.dll
.ELSE
OUT2LIB+=win32$/binaries$/*.lib
OUT2BIN+=win32$/binaries$/*.dll
.ENDIF
.ELSE
OUT2LIB+=src$/.libs$/libxmlsec1.a src$/nss$/.libs$/libxmlsec1-nss.a
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk


