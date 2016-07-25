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

PRJNAME=nss
TARGET=nss

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

.IF "$(ENABLE_NSS_MODULE)"!="YES" || "$(SYSTEM_NSS)"=="YES"

all:
    @echo "NSS will not be built because ENABLE_NSS_MODULE='$(ENABLE_NSS_MODULE)' and SYSTEM_NSS='$(SYSTEM_NSS)'"

.ELSE

TARFILE_NAME=nss-3.25-with-nspr-4.12
TARFILE_MD5=4ec9a36c0f7c9360b149491c013b8d50
TARFILE_ROOTDIR=nss-3.25
PATCH_FILES=nss.patch

.IF "$(OS)"=="MACOSX"
MACOS_SDK_DIR=$(SDK_PATH)
.EXPORT : MACOS_SDK_DIR
PATCH_FILES+=nss_macosx.patch
.ENDIF # "$(OS)"=="MACOSX"

.IF "$(debug)" != ""
.ELSE
BUILD_OPT=1
.EXPORT: BUILD_OPT
.ENDIF

.IF "$(BUILD64)"=="1"
# force the 64-bit build mode for 64bit targets
USE_64:=1
.EXPORT : USE_64
.ENDIF # "$(BUILD64)"=="1"

OUT2LIB=dist$/out$/lib$/*$(DLLPOST)

BUILD_DIR=nss
BUILD_ACTION= $(GNUMAKE) nss_build_all
#See #i105566# && moz#513024#
.IF "$(OS)"=="LINUX"
BUILD_ACTION+=FREEBL_NO_DEPEND=1 FREEBL_LOWHASH=1 NSS_DISABLE_GTESTS=1
PATCH_FILES+=nss_linux.patch
.ENDIF

.IF "$(OS)"=="FREEBSD"
BUILD_ACTION+=NSS_DISABLE_GTESTS=1
.ENDIF


.IF "$(GUI)"=="WNT"

.IF "$(COM)"=="GCC"

PATCH_FILES+=nss.patch.mingw

moz_build:=$(shell cygpath -p $(MOZILLABUILD))
PATH!:=$(moz_build)/bin:$(PATH)

nss_CC=$(CC)
nss_CXX=$(CXX)
.IF "$(MINGW_SHARED_GCCLIB)"=="YES"
nss_CC+=-shared-libgcc
nss_CXX+=-shared-libgcc
.ENDIF

nss_LIBS=
.IF "$(MINGW_SHARED_GXXLIB)"=="YES"
nss_LIBS+=$(MINGW_SHARED_LIBSTDCPP)
.ENDIF


BUILD_DIR=nss
BUILD_ACTION= NSS_DISABLE_GTESTS=1 NS_USE_GCC=1 CC="$(nss_CC)" CXX="$(nss_CXX)" OS_LIBS="$(nss_LIBS)" OS_TARGET=WIN95 _WIN32_IE=0x500 PATH="$(PATH)" DEFINES=-D_WIN32_IE=0x500 $(GNUMAKE) nss_build_all

OUT2LIB= \
    dist$/out$/lib$/libnspr4.a \
    dist$/out$/lib$/libnss3.a \
    dist$/out$/lib$/libnssdbm3.a \
    dist$/out$/lib$/libnssutil3.a \
    dist$/out$/lib$/libplc4.a \
    dist$/out$/lib$/libplds4.a \
    dist$/out$/lib$/libsmime3.a \
    dist$/out$/lib$/libsoftokn3.a \
    dist$/out$/lib$/libsqlite3.a \
    dist$/out$/lib$/libssl3.a

.ELSE			# "$(COM)"=="GCC"
MOZ_MSVCVERSION= 9
.EXPORT : MOZ_MSVCVERSION
moz_build:=$(shell cygpath -p $(MOZILLABUILD))

#Using WINNT will cause at least that nspr4.dll, plc4.dll, plds4.dll 
#become libnspr4.dll, libplc4.dll, libplds4.dll
#WINNT causes the use of Microsoft fibers (specific to Windows NT and Windows 2000). 
#OS_TARGET= WINNT
OS_TARGET=WIN95
.EXPORT : OS_TARGET

# make use of stlport headerfiles
EXT_USE_STLPORT=TRUE

#To build nss one has to call "make nss_build_all" in 
#nss
NSS_BUILD_DIR= $(subst,\,/ $(PWD)/$(MISC)/build/$(TARFILE_ROOTDIR)/nss)
BUILD_ACTION= PATH="$(PATH):$(moz_build)/msys/bin:$(moz_build)/moztools/bin" && $(subst,/,$/ $(MOZILLABUILD)/msys/bin/bash) -i \
    -c "cd $(NSS_BUILD_DIR) && make nss_build_all NSS_DISABLE_GTESTS=1"

OUT2LIB= \
    dist$/out$/lib$/nspr4.lib \
    dist$/out$/lib$/nss3.lib \
    dist$/out$/lib$/nssdbm3.lib \
    dist$/out$/lib$/nssutil3.lib \
    dist$/out$/lib$/plc4.lib \
    dist$/out$/lib$/plds4.lib \
    dist$/out$/lib$/smime3.lib \
    dist$/out$/lib$/softokn3.lib \
    dist$/out$/lib$/sqlite3.lib \
    dist$/out$/lib$/ssl3.lib

.ENDIF			# "$(COM)"=="GCC"

OUT2BIN=dist$/out$/lib$/*$(DLLPOST)
.ENDIF			# "$(GUI)"=="WNT"


OUTDIR2INC=dist$/public$/nss dist$/out$/include

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk

.ENDIF
