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

PRJNAME=nss
TARGET=nss

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

.IF "$(ENABLE_NSS_MODULE)"!="YES"
all:
    @echo "NSS will not be built. ENABLE_NSS_MODULE is '$(ENABLE_NSS_MODULE)'"
.ENDIF	

TARFILE_NAME=nss-3.12.8-with-nspr-4.8.6
TARFILE_MD5=71474203939fafbe271e1263e61d083e
TARFILE_ROOTDIR=nss-3.12.8
PATCH_FILES=nss.patch nss.aix.patch

.IF "$(OS)"=="MACOSX"
PATCH_FILES+=nss_macosx.patch
.ENDIF # "$(OS)"=="MACOSX"

.IF "$(debug)" != ""
.ELSE
BUILD_OPT=1
.EXPORT: BUILD_OPT
.ENDIF

.IF "$(GUI)"=="UNX"
.IF "$(OS)$(COM)"=="LINUXGCC"
.IF "$(BUILD64)"=="1"
# force 64-bit buildmode
USE_64:=1
.EXPORT : USE_64
.ENDIF			# "$(CPU)"=="X"
.ENDIF                  # "$(OS)$(COM)"=="LINUXGCC"

.IF "$(OS)$(COM)"=="FREEBSDGCC"
.IF "$(CPU)"=="X"
# force 64-bit buildmode
USE_64:=1
.EXPORT : USE_64
.ENDIF			# "$(CPU)"=="X"
.ENDIF                  # "$(OS)$(COM)"=="LINUXGCC"

.IF "$(OS)"=="MACOSX"
.IF "$(EXTRA_CFLAGS)"!=""
CPP:=gcc -E $(EXTRA_CFLAGS)
CXX:=g++ $(EXTRA_CFLAGS)
CC:=gcc $(EXTRA_CFLAGS)
MACOS_SDK_DIR*=$(MACDEVSDK)
.EXPORT : CPP MACOS_SDK_DIR
.ENDIF # "$(EXTRA_CFLAGS)"!=""
.ENDIF # "$(OS)"=="MACOSX"

OUT2LIB=mozilla$/dist$/out$/lib$/*$(DLLPOST)

BUILD_DIR=mozilla$/security$/nss
BUILD_ACTION= $(GNUMAKE) nss_build_all
#See #i105566# && moz#513024#
.IF "$(OS)"=="LINUX"
BUILD_ACTION+=FREEBL_NO_DEPEND=1
.ENDIF

.ENDIF			# "$(GUI)"=="UNX"


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
nss_LIBS+=-lstdc++_s
.ENDIF


BUILD_DIR=mozilla$/security$/nss
BUILD_ACTION=NS_USE_GCC=1 CC="$(nss_CC)" CXX="$(nss_CXX)" OS_LIBS="$(nss_LIBS)" OS_TARGET=WIN95 _WIN32_IE=0x500 PATH="$(PATH)" DEFINES=-D_WIN32_IE=0x500 $(GNUMAKE) nss_build_all

OUT2LIB= \
    mozilla$/dist$/out$/lib$/libnspr4.a \
    mozilla$/dist$/out$/lib$/libnss3.a \
    mozilla$/dist$/out$/lib$/libnssdbm3.a \
    mozilla$/dist$/out$/lib$/libnssutil3.a \
    mozilla$/dist$/out$/lib$/libplc4.a \
    mozilla$/dist$/out$/lib$/libplds4.a \
    mozilla$/dist$/out$/lib$/libsmime3.a \
    mozilla$/dist$/out$/lib$/libsoftokn3.a \
    mozilla$/dist$/out$/lib$/libsqlite3.a \
    mozilla$/dist$/out$/lib$/libssl3.a

.ELSE			# "$(COM)"=="GCC"
MOZ_MSVCVERSION= 9
.EXPORT : MOZ_MSVCVERSION
moz_build:=$(shell cygpath -p $(MOZILLABUILD))
PATH!:=$(moz_build)/msys/bin:$(moz_build)/moztools/bin:$(PATH)
.EXPORT : PATH

#Using WINNT will cause at least that nspr4.dll, plc4.dll, plds4.dll 
#become libnspr4.dll, libplc4.dll, libplds4.dll
#WINNT causes the use of Microsoft fibers (specific to Windows NT and Windows 2000). 
#OS_TARGET= WINNT
OS_TARGET=WIN95
.EXPORT : OS_TARGET

#To build nss one has to call "make nss_build_all" in 
#mozilla/security/nss
NSS_BUILD_DIR= $(subst,\,/ $(PWD)/$(MISC)/build/$(TARFILE_ROOTDIR)/mozilla/security/nss)
BUILD_ACTION= $(subst,/,$/ $(MOZILLABUILD)/msys/bin/bash) -i \
    -c "cd $(NSS_BUILD_DIR) && make nss_build_all"

OUT2LIB= \
     mozilla$/dist$/out$/lib$/nspr4.lib \
     mozilla$/dist$/out$/lib$/nss3.lib \
     mozilla$/dist$/out$/lib$/nssdbm3.lib \
     mozilla$/dist$/out$/lib$/nssutil3.lib \
     mozilla$/dist$/out$/lib$/plc4.lib \
     mozilla$/dist$/out$/lib$/plds4.lib \
     mozilla$/dist$/out$/lib$/smime3.lib \
     mozilla$/dist$/out$/lib$/softokn3.lib \
     mozilla$/dist$/out$/lib$/sqlite3.lib \
     mozilla$/dist$/out$/lib$/ssl3.lib

.ENDIF			# "$(COM)"=="GCC"

OUT2BIN=mozilla$/dist$/out$/lib$/*$(DLLPOST)
.ENDIF			# "$(GUI)"=="WNT"


OUTDIR2INC=mozilla$/dist$/public$/nss mozilla$/dist$/out$/include

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk


