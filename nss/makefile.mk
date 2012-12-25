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

.IF "$(SYSTEM_NSS)"=="YES"
all:
	@echo "NSS will not be built. Using system one instead."
.ENDIF

.IF "$(OS)" == "ANDROID" || "$(OS)" == "IOS"
@all:
	@echo "FIXME: NSS module not buildable yet on this platform"
.ENDIF

VER_MAJOR=3
VER_MINOR=13
VER_PATCH=5

TARFILE_NAME=nss-$(VER_MAJOR).$(VER_MINOR).$(VER_PATCH)-with-nspr-4.9.1
TARFILE_MD5=a0a861f539f0e7a91d05e6b9457e4db1
TARFILE_ROOTDIR=nss-$(VER_MAJOR).$(VER_MINOR).$(VER_PATCH)
PATCH_FILES=nss.patch nss.aix.patch nss-config.patch \
    nss-3.13.5-zlib-werror.patch
# nss-3.13.5-zlib-werror.patch: avoid -Werror=implicit-function-declaration for
# lseek, read, write

.IF "$(OS)"=="MACOSX"
PATCH_FILES+=nss_macosx.patch
.ENDIF # "$(OS)"=="MACOSX"

# For a MSVC build, not exporting BUILD_OPT causes the produced DLLs
# to use the debug CRT. (The exact mechanism that causes this to
# happen is a bit of a mystery...) That is confusing and wrong, as
# nothing in LO otherwise uses that. It also makes testing a build
# much harder for me at least, as I do that in a fairly pristine
# virtual machine with no MSVC debugging runtime available. (The
# normal CRT is bundled in the LO installer.)
.IF "$(debug)" != "" && "$(OS)$(COM)" != "WNTMSC"
.ELSE
BUILD_OPT=1
.EXPORT: BUILD_OPT
.ENDIF

.IF "$(OS)" == "MACOSX"
my_prefix=/@.__________________________________________________$(EXTRPATH)
.ELSE
my_prefix=$(OUTDIR)
.END

CONFIGURE_ACTION=mozilla/nsprpub/configure --prefix=$(my_prefix) --includedir=$(OUTDIR)/inc/mozilla/nspr

.IF "$(CROSS_COMPILING)"=="YES"
CONFIGURE_ACTION+=--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)
.ENDIF

.IF "$(OS)$(COM)$(CPUNAME)"=="WNTMSCX86_64"
CONFIGURE_ACTION+=--enable-64bit
.ENDIF

CONFIGURE_ACTION+= ; \
    sed -e 's\#@prefix@\#$(OUTDIR)\#' -e 's\#@includedir@\#$(OUTDIR)/inc/mozilla/nss\#' -e 's\#@MOD_MAJOR_VERSION@\#$(VER_MAJOR)\#' -e 's\#@MOD_MINOR_VERSION@\#$(VER_MINOR)\#' -e 's\#@MOD_PATCH_VERSION@\#$(VER_PATCH)\#' mozilla/security/nss/nss-config.in > mozilla/security/nss/nss-config ; \
    chmod a+x mozilla/security/nss/nss-config

.IF "$(OS)"!="WNT"
.IF "$(OS)$(COM)"=="LINUXGCC"
.IF "$(BUILD64)"=="1"
# force 64-bit buildmode
USE_64:=1
.EXPORT : USE_64
.ENDIF
.ENDIF

.IF "$(OS)$(COM)"=="MACOSXGCC"
.IF "$(BUILD64)"=="1"
# force 64-bit buildmode
USE_64:=1
.EXPORT : USE_64
.ENDIF
.ENDIF

.IF "$(OS)$(COM)"=="FREEBSDGCC"
.IF "$(CPUNAME)"=="X86_64"
# force 64-bit buildmode
USE_64:=1
.EXPORT : USE_64
.ENDIF
.ENDIF

.IF "$(OS)"=="MACOSX"
MACOS_SDK_DIR:=$(MACOSX_SDK_PATH)
.EXPORT : MACOS_SDK_DIR
.ENDIF # "$(OS)"=="MACOSX"

OUT2LIB=mozilla/dist/out/lib/*$(DLLPOST) mozilla/dist/out/lib/libcrmf.a

OUT2BIN=config/nspr-config mozilla/security/nss/nss-config

BUILD_DIR=mozilla/security/nss
BUILD_ACTION= $(GNUMAKE) nss_build_all -j1

.IF "$(CROSS_COMPILING)"=="YES"

.IF "$(OS)-$(CPUNAME)"="MACOSX-POWERPC"
# Hardcode this for now... need to add more when/if cross-compiling to
# other desktop OSes, yeah, this is silly, but the nss build mechanism
# is a bit messy, I could not figure out how to get it to get CPU_ARCH
# automatically when cross-compiling
BUILD_ACTION+= CPU_ARCH=ppc
.ENDIF

# When cross-compiling need to use a nsinstall built for the build
# platform, so yeah, whole nss built for the build platform just for
# that... But oh well, nss is small compared to LO;)
BUILD_ACTION+= NSINSTALL=$(SRC_ROOT)/nss/$(INPATH_FOR_BUILD)/misc/build/$(TARFILE_ROOTDIR)/mozilla/security/coreconf/nsinstall/out/nsinstall

.ENDIF

#Note: with the new version the libfreebl3.so gets built in a way that does
# not conflict with the system one on Linux automatically;
# it is no longer necessary to add a workaround for #i105566# && moz#513024#
.IF "$(OS)$(COM)"=="SOLARISGCC"
BUILD_ACTION+=NS_USE_GCC=1
.ENDIF

# Otherwise, LibreOffice.app/Contents/MacOS/python -c 'import uno' fails to
# dlopen LibreOffice.app/Contents/MacOS/libpyuno.dylib (at
# pyuno/source/module/pyuno_dlopenwrapper.c:73) with dlerror "Symbol not found:
# _sqlite3_wal_checkpoint; Referenced from: /System/Library/Frameworks/
# CoreServices.framework/Versions/A/Frameworks/CFNetwork.framework/Versions/A/
# CFNetwork; Expected in: [...]/LibreOffice.app/Contents/MacOS/libsqlite3.dylib;
# in /System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/
# CFNetwork.framework/Versions/A/CFNetwork":
.IF "$(OS)" == "MACOSX" && "$(MAC_OS_X_VERSION_MIN_REQUIRED)" >= "1060"
BUILD_ACTION += NSS_USE_SYSTEM_SQLITE=1
.END

.ENDIF			# "$(OS)"!="WNT"


.IF "$(OS)"=="WNT"

.IF "$(COM)"=="GCC"

PATCH_FILES += \
	       nspr-4.9-build.patch \
	       nss-3.13.3-build.patch \
	       nss.patch.mingw \


PATH!:=$(NSSBUILDTOOLS)/bin:$(PATH)

nss_CC=$(CC)
nss_CXX=$(CXX)
.IF "$(MINGW_SHARED_GCCLIB)"=="YES"
nss_CC+=-shared-libgcc
nss_CXX+=-shared-libgcc
.ENDIF

nss_LIBS=-ladvapi32 -lws2_32 -lmswsock -lwinmm
.IF "$(MINGW_SHARED_GXXLIB)"=="YES"
nss_LIBS+=$(MINGW_SHARED_LIBSTDCPP)
.ENDIF
nss_LDFLAGS=

OS_TARGET=WINNT
.EXPORT : OS_TARGET

BUILD_DIR=mozilla/security/nss
BUILD_ACTION=$(GNUMAKE) NS_USE_GCC=1 CC="$(nss_CC)" CXX="$(nss_CXX)" OS_LIBS="$(nss_LIBS)" PATH="$(PATH)" NSPR_CONFIGURE_OPTS="--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM) --enable-shared --disable-static" LDFLAGS="$(nss_LDFLAGS)" RANLIB="$(RANLIB)" RC="$(WINDRES)" OS_RELEASE=5.0 NSINSTALL="$(PYTHON_FOR_BUILD) $(SRC_ROOT)/nss/nsinstall.py" IMPORT_LIB_SUFFIX=dll.a nss_build_all

OUT2LIB= \
    mozilla/nsprpub/out/pr/src/libnspr4.dll.a \
    mozilla/dist/out/lib/libnss3.dll.a \
    mozilla/dist/out/lib/libnssdbm3.dll.a \
    mozilla/dist/out/lib/libnssutil3.dll.a \
    mozilla/nsprpub/out/lib/libc/src/libplc4.dll.a \
    mozilla/nsprpub/out/lib/ds/libplds4.dll.a \
    mozilla/dist/out/lib/libsmime3.dll.a \
    mozilla/dist/out/lib/libsoftokn3.dll.a \
    mozilla/dist/out/lib/libsqlite3.dll.a \
    mozilla/dist/out/lib/libssl3.dll.a

.ELSE			# "$(COM)"=="GCC"
MOZ_MSVCVERSION= 9
.EXPORT : MOZ_MSVCVERSION
nss_build:=$(shell cygpath -p $(NSSBUILDTOOLS))

#Using WINNT will cause at least that nspr4.dll, plc4.dll, plds4.dll 
#become libnspr4.dll, libplc4.dll, libplds4.dll
#WINNT causes the use of Microsoft fibers (specific to Windows NT and Windows 2000). 
#OS_TARGET= WINNT
OS_TARGET=WIN95
.EXPORT : OS_TARGET

#To build nss one has to call "make nss_build_all" in 
#mozilla/security/nss

.IF "$(CPUNAME)"=="X86_64"
PASS_USE_64=USE_64=1
.ENDIF

NSS_BUILD_DIR=$(ABS_PACKAGE_DIR)/$(TARFILE_ROOTDIR)/mozilla/security/nss
BUILD_ACTION= PATH="$(nss_build)/msys/bin:$(nss_build)/moztools/bin:$(PATH)" && $(NSSBUILDTOOLS)/msys/bin/bash -i \
    -c "cd $(NSS_BUILD_DIR) && make $(PASS_USE_64) nss_build_all"

OUT2LIB= \
     mozilla/dist/out/lib/nspr4.lib \
     mozilla/dist/out/lib/nss3.lib \
     mozilla/dist/out/lib/nssdbm3.lib \
     mozilla/dist/out/lib/nssutil3.lib \
     mozilla/dist/out/lib/plc4.lib \
     mozilla/dist/out/lib/plds4.lib \
     mozilla/dist/out/lib/smime3.lib \
     mozilla/dist/out/lib/softokn3.lib \
     mozilla/dist/out/lib/sqlite3.lib \
     mozilla/dist/out/lib/ssl3.lib

.ENDIF			# "$(COM)"=="GCC"

OUT2BIN=mozilla/dist/out/lib/*$(DLLPOST) \
     config/nspr-config \
     mozilla/security/nss/nss-config

.ENDIF			# "$(OS)"=="WNT"


OUTDIR2INC=mozilla/dist/public/nss mozilla/dist/out/include

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk


