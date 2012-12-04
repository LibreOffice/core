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

PRJNAME=ooo_mozab
TARGET=ooo_mozab

# --- Settings -----------------------------------------------------

.INCLUDE :    settings.mk

# --- Files --------------------------------------------------------

# ----- pkg-config start -------
.INCLUDE .IGNORE : pkgroot.mk
.IF "$(PKGCONFIG_ROOT)"!=""
PKG_CONFIG:=$(PKGCONFIG_ROOT)$/bin$/pkg-config
PKG_CONFIG_PATH:=$(PKGCONFIG_ROOT)$/lib$/pkgconfig
.IF "$(LD_LIBRARY_PATH)"=="" # assume empty implies unset
LD_LIBRARY_PATH!:=$(PKGCONFIG_ROOT)$/lib
.ELSE
LD_LIBRARY_PATH!:=$(LD_LIBRARY_PATH)$(PATH_SEPERATOR)$(PKGCONFIG_ROOT)$/lib
.ENDIF
.EXPORT : PKG_CONFIG_PATH PKG_CONFIG LD_LIBRARY_PATH
.ENDIF          # "$(PKGCONFIG_ROOT)"!=""

# reduce prerequisites by disabling mozilla binary
DISABLE_MOZ_EXECUTABLE=TRUE
.EXPORT : DISABLE_MOZ_EXECUTABLE


# ----- pkg-config end -------

MOZILLA_VERSION=1.1.14
TARFILE_NAME=seamonkey-$(MOZILLA_VERSION).source
TARFILE_MD5=a169ab152209200a7bad29a275cb0333

TARFILE_ROOTDIR=mozilla
PATCH_FILES = \
    seamonkey-source-$(MOZILLA_VERSION).patch \
    patches/dtoa.patch \
    patches/respect_disable_pango.patch \
    patches/link_fontconfig.patch \
    patches/brokenmakefile.patch \
    patches/libpr0n_build_fix.patch \
    patches/clang_add_nsCaseInsensitiveStringComparator_default_constructor.patch \
    patches/clang_missing_this_pointers.patch \
    patches/seamonkey-1.1.14.source-macosx10.6and7.patch \

# This file is needed for the W32 build when BUILD_MOZAB is set
# (currently only vc8/vs2005 is supported when BUILD_MOZAB is set)
MOZTOOLS_ZIPFILE_NAME*=vc8-moztools.zip

ADDITIONAL_FILES=mailnews$/addrbook$/src$/nsAbMD5sum.cpp

CONFIGURE_DIR=

MOZILLA_CONFIGURE_FLAGS +=  --disable-tests \
                --enable-application=suite \
                --enable-ldap \
                --enable-crypto \
                --enable-optimize \
                --enable-strip \
                --disable-profilelocking \
                --disable-activex \
                --disable-activex-scripting \
                --disable-gnomevfs \
                --disable-debug \
                --disable-xprint \
                --disable-postscript \
                --without-system-zlib \
                --with-system-nss \
                --with-system-nspr \
                --disable-installer \
                --disable-accessibility \
                --disable-xpfe-components \
                --disable-mathml \
                --disable-oji \
                --disable-profilesharing \
                --disable-boehm \
                --disable-jsloader \
                --disable-canvas \
                --disable-freetype2 \
                --disable-gnomeui \
                --disable-image-encoders \
                --disable-plugins \
                --disable-printing \
                --disable-pango \
                --enable-extensions="pref"

.IF "$(SYSTEM_NSS)"=="NO"

MOZILLA_CONFIGURE_FLAGS += \
                --with-nss-prefix=$(OUTDIR) \
                --with-nspr-prefix=$(OUTDIR)
.ENDIF

#disable profilelocking to share profile with mozilla
#disable activex and activex-scripting to remove the dependence of Microsoft_SDK\src\mfc\atlbase.h
#disable gnomevfs to remove the needed of gnome develop files
#disable others to save build times

# create a objdir build = build files in a seperate directory, not in the sourcetree directly
CONFIGURE_DIR=$(CPU)_objdir
BUILD_DIR=$(CONFIGURE_DIR)
MOZ_CROSSCOMPILE=CROSS_COMPILE=1 CC="$(CC) -arch $(MOZ_ARCH)" CXX="$(CXX) -arch $(MOZ_ARCH)" AR=ar

CONFIGURE_ACTION=$(null,$(MOZ_ARCH) $(NULL) $(MOZ_CROSSCOMPILE)) ../configure $(MOZILLA_CONFIGURE_FLAGS)

BUILD_ACTION:=$(GNUMAKE) -j$(EXTMAXPROCESS)

MOZDIR=$(MISC)$/build$/seamonkey
MOZTARGET=$(OS)$(COM)$(CPU)

# "Our" build environment uses "NO" for the environment variable that
# indicate if system libraries should be used, the mozilla build uses
# "" in this case. This conflicts (at least for W32) with mozilla (1.7b)
# because it disables the library checks for msvc so that
# --without-system-* is not evaluated. To build the included libraries
# the affected variables have to be empty and not NO.
.IF "$(SYSTEM_ZLIB)"=="NO"
SYSTEM_ZLIB:=
.EXPORT : SYSTEM_ZLIB
.ENDIF
.IF "$(SYSTEM_JPEG)"=="NO"
SYSTEM_JPEG:=
.EXPORT : SYSTEM_JPEG
.ENDIF

CC:=cl.exe
CXX:=cl.exe

# Variables to install/use our own wintools
MOZTOOLS_EXTRACT:=$(MISC)$/build$/moztools
MOZ_TOOLS_DOS:=$(shell @cygpath -ad "$(MISC)")\build\moztools\vc8-moztools
PATH!:=$(shell @cygpath $(MOZ_TOOLS_DOS))/bin:$(PATH)

MOZ_TOOLS:=$(subst,\,/ $(MOZ_TOOLS_DOS))

.EXPORT : PATH MOZ_TOOLS

# --- Targets ------------------------------------------------------

.IF "$(BUILD_MOZAB)"==""
all:
    @echo "Never Build Mozilla but copy runtime files $(MSVC80_DLL_DIR) ."
    @echo "Never Build Mozilla but copy runtime files."
    @@-$(MKDIR) $(OUT)$/zipped
    $(COPY) $(TARFILE_LOCATION)$/$(MOZ_ZIP_INC) $(PRJ)$/zipped$/
    $(COPY) $(TARFILE_LOCATION)$/$(MOZ_ZIP_LIB) $(PRJ)$/zipped$/
    $(COPY) $(TARFILE_LOCATION)$/$(MOZ_ZIP_RUNTIME) $(PRJ)$/zipped$/
.IF "$(EXTMSV80)"!=""
    $(COPY) $(MSVC80_MNFST) $(EXTMSV80)$/Microsoft.VC80.CRT.manifest
    $(COPY) $(MSVC80_DLL_DIR)$/msvcp80.dll $(EXTMSV80)$/
    $(COPY) $(MSVC80_DLL_DIR)$/msvcr80.dll $(EXTMSV80)$/
.ENDIF    
.ENDIF    

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

NEEDWINTOOLSFLAGFILE:=$(MISC)$/build$/moztools.complete

ALLTAR: $(NEEDWINTOOLSFLAGFILE) \
    $(MISC)$/remove_build.flag \
    extract_mozab_files

$(PACKAGE_DIR)$/$(UNTAR_FLAG_FILE) : $(MISC)$/remove_build.flag

# Since you never know what will be in a patch (for example, it may already
# patch at configure level), we remove the entire package directory if a patch
# is newer.
$(MISC)$/remove_build.flag : $(PATCH_FILES)
    $(REMOVE_PACKAGE_COMMAND)
    $(TOUCH) $(MISC)$/remove_build.flag

# Unpack/setup Windows build tools
$(PACKAGE_DIR)$/$(UNTAR_FLAG_FILE) : $(MISC)$/build$/moztools.complete

$(MISC)$/build$/moztools.unpack : $(PRJ)$/download$/$(MOZTOOLS_ZIPFILE_NAME)
    -$(RENAME) $(MOZTOOLS_EXTRACT) $(MOZTOOLS_EXTRACT)_removeme
    -rm -rf $(MOZTOOLS_EXTRACT)_removeme
    @-$(MKDIRHIER) $(MOZTOOLS_EXTRACT)
    unzip $(PRJ)$/download$/$(MOZTOOLS_ZIPFILE_NAME) -d $(MOZTOOLS_EXTRACT)
# chmod is also needed for W32-4nt build (when cygwin unzip is used)
    -chmod -R +x $(MOZTOOLS_EXTRACT)$/vc8-moztools$/bin
    $(TOUCH) $(MISC)$/build$/moztools.unpack

$(MISC)$/build$/moztools.complete : $(MISC)$/build$/moztools.unpack
    $(TOUCH) $(MISC)$/build$/moztools.complete

zip:    \
    $(MISC)$/CREATETARBALL

.INCLUDE : extractfiles.mk
