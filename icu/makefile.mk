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

PRJNAME=icu
TARGET=so_icu

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=icu4c-49_1_1-src
TARFILE_MD5=7c53f83e0327343f4060c0eb83842daf
TARFILE_ROOTDIR=icu

#icu4c.8320.freeserif.crash.patch, see
#http://bugs.icu-project.org/trac/ticket/8320 for crash with FreeSerif

#icu4c.7601.Indic-ccmp.patch
#http://bugs.icu-project.org/trac/ticket/7601 ccmp feature for Indic Languages

#icu4c.8198.revert.icu5431.patch
#http://bugs.icu-project.org/trac/ticket/8198 rendering with 0D30 and 0D31

PATCH_FILES=\
    icu4c-build.patch \
    icu4c.8320.freeserif.crash.patch \
    icu4c.8198.revert.icu5431.patch \
    icu4c.7601.Indic-ccmp.patch \
    icu4c-aix.patch \
    icu4c-wchar_t.patch \
    icu4c-warnings.patch \
    icu4c.9313.cygwin.patch \
    icu4c-macosx.patch \
    icu4c-interlck.patch \
    icu4c-solarisgcc.patch \

.IF "$(OS)"=="ANDROID"
PATCH_FILES+=\
    icu4c-android.patch
.ELSE
PATCH_FILES+=\
    icu4c-rpath.patch
.ENDIF

.IF "$(GUI)"=="UNX"

.IF "$(SYSBASE)"!=""
icu_CFLAGS+=-I$(SYSBASE)$/usr$/include
.IF "$(COMNAME)"=="sunpro5"
icu_CFLAGS+=$(C_RESTRICTIONFLAGS)
.ENDIF			# "$(COMNAME)"=="sunpro5"
# add SYSBASE libraries and make certain that they are found *after* the
# icu build internal libraries - in case that icu is available in SYSBASE
# as well
icu_LDFLAGS+= -L../lib  -L../../lib -L../stubdata -L../../stubdata  -L$(SYSBASE)$/usr$/lib
.ENDIF			# "$(SYSBASE)"!=""

.IF "$(OS)"=="MACOSX"
.IF "$(EXTRA_CFLAGS)"!=""
CPP:=gcc -E $(EXTRA_CFLAGS)
CXX:=g++ $(EXTRA_CFLAGS)
CC:=gcc $(EXTRA_CFLAGS)
.EXPORT : CPP
.ENDIF # "$(EXTRA_CFLAGS)"!=""
.ENDIF # "$(OS)"=="MACOSX"

.IF "$(debug)" != ""
icu_CFLAGS+=-g $(ARCH_FLAGS)
icu_CXXFLAGS+=-g $(ARCH_FLAGS)
.ELIF "$(COM)"=="GCC"
icu_CFLAGS+=-O2 -fno-strict-aliasing $(ARCH_FLAGS)
icu_CXXFLAGS+=-O2 -fno-strict-aliasing $(ARCH_FLAGS)
.ELSE
icu_CFLAGS+=-O $(ARCH_FLAGS)
icu_CXXFLAGS+=-O $(ARCH_FLAGS)
.ENDIF
icu_LDFLAGS+=$(EXTRA_LINKFLAGS)

# until someone introduces SOLARIS 64-bit builds
.IF "$(OS)"=="SOLARIS"
DISABLE_64BIT=--enable-64bit-libs=no
.ENDIF			# "$(OS)"=="SOLARIS"

.IF "$(OS)"=="AIX"
DISABLE_64BIT=--enable-64bit-libs=no
LDFLAGSADD+=$(LINKFLAGS) $(LINKFLAGSRUNPATH_OOO)
.ENDIF                  # "$(OS)"=="AIX"

.IF "$(HAVE_LD_HASH_STYLE)"  == "TRUE"
LDFLAGSADD += -Wl,--hash-style=$(WITH_LINKER_HASH_STYLE)
.ENDIF

.IF "$(OS)"=="IOS"
# Let's try this...
icu_CFLAGS+=-DUCONFIG_NO_FILE_IO
.ENDIF

.IF "$(OS)"=="ANDROID"
# Problems with uint64_t on Android unless disabling strictness
DISABLE_STRICT=--disable-strict
icu_CFLAGS+=-fno-omit-frame-pointer
icu_CXXFLAGS+=-fno-omit-frame-pointer
.ENDIF

.IF "$(OS)"=="IOS" || "$(OS)"=="ANDROID"
# Problems on Android unless disabling dynamic loading
DISABLE_DYLOAD=--disable-dyload
.ENDIF

.IF "$(HAVE_LD_BSYMBOLIC_FUNCTIONS)"  == "TRUE"
LDFLAGSADD += -Wl,-Bsymbolic-functions -Wl,--dynamic-list-cpp-new -Wl,--dynamic-list-cpp-typeinfo
.ENDIF

CONFIGURE_DIR=source

.IF "$(OS)"=="IOS"
STATIC_OR_SHARED=--enable-static --disable-shared
.ELSE
STATIC_OR_SHARED=--disable-static --enable-shared
.ENDIF

.IF "$(CROSS_COMPILING)"=="YES"
# We require that the cross-build-toolset target from the top Makefile(.in) has bee built
BUILD_AND_HOST=--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM) --with-cross-build=$(posix_PWD)/$(INPATH_FOR_BUILD)/misc/build/icu/source
.ENDIF

.IF "$(OS)"=="ANDROID"
LIBRARY_SUFFIX= --with-library-suffix=lo
icu_LDFLAGS+=-lgnustl_shared
.ENDIF

CONFIGURE_ACTION+=sh -c 'CPPFLAGS="$(EXTRA_CDEFS)" CFLAGS="$(icu_CFLAGS)" CXXFLAGS="$(icu_CXXFLAGS)" LDFLAGS="$(icu_LDFLAGS) $(LDFLAGSADD)" \
./configure --enable-layout --disable-samples $(STATIC_OR_SHARED) $(BUILD_AND_HOST) $(DISABLE_64BIT) $(DISABLE_STRICT) $(DISABLE_DYLOAD) $(LIBRARY_SUFFIX)'

.IF "$(OS)" == "MACOSX"
CONFIGURE_ACTION += \
    --prefix=/@.__________________________________________________$(EXTRPATH)
.END

CONFIGURE_FLAGS=

# Use of
# CONFIGURE_ACTION=sh -c 'CFLAGS=-O CXXFLAGS=-O ./configure'
# CONFIGURE_FLAGS=--enable-layout --enable-static --enable-shared=yes --enable-64bit-libs=no
# doesn't work as it would result in
# sh -c 'CFLAGS=-O CXXFLAGS=-O ./configure' --enable-layout ...
# note the position of the single quotes.

BUILD_DIR=$(CONFIGURE_DIR)
BUILD_ACTION=$(AUGMENT_LIBRARY_PATH) $(GNUMAKE) -j$(EXTMAXPROCESS)
.IF "$(OS)"=="IOS"
OUT2LIB= \
    $(BUILD_DIR)$/lib$/libicudata.a \
    $(BUILD_DIR)$/lib$/libicuuc.a \
    $(BUILD_DIR)$/lib$/libicui18n.a \
    $(BUILD_DIR)$/lib$/libicule.a \
    $(BUILD_DIR)$/lib$/libicutu.a
.ELIF "$(OS)"=="ANDROID"
BUILD_ACTION+= && cat uconfig.h.prepend common/unicode/uconfig.h >common/unicode/uconfig.h.new && mv common/unicode/uconfig.h.new common/unicode/uconfig.h
OUT2LIB= \
    $(BUILD_DIR)$/lib$/libicudatalo.so \
    $(BUILD_DIR)$/lib$/libicuuclo.so \
    $(BUILD_DIR)$/lib$/libicui18nlo.so \
    $(BUILD_DIR)$/lib$/libiculelo.so \
    $(BUILD_DIR)$/lib$/libicutulo.so
.ELSE
OUT2LIB= \
    $(BUILD_DIR)$/lib$/libicudata$(DLLPOST).$(ICU_MAJOR).$(ICU_MINOR).$(ICU_MICRO) \
    $(BUILD_DIR)$/lib$/libicudata$(DLLPOST).$(ICU_MAJOR) \
    $(BUILD_DIR)$/lib$/libicudata$(DLLPOST) \
    $(BUILD_DIR)$/lib$/libicuuc$(DLLPOST).$(ICU_MAJOR).$(ICU_MINOR).$(ICU_MICRO) \
    $(BUILD_DIR)$/lib$/libicuuc$(DLLPOST).$(ICU_MAJOR) \
    $(BUILD_DIR)$/lib$/libicuuc$(DLLPOST) \
    $(BUILD_DIR)$/lib$/libicui18n$(DLLPOST).$(ICU_MAJOR).$(ICU_MINOR).$(ICU_MICRO) \
    $(BUILD_DIR)$/lib$/libicui18n$(DLLPOST).$(ICU_MAJOR) \
    $(BUILD_DIR)$/lib$/libicui18n$(DLLPOST) \
    $(BUILD_DIR)$/lib$/libicule$(DLLPOST).$(ICU_MAJOR).$(ICU_MINOR).$(ICU_MICRO) \
    $(BUILD_DIR)$/lib$/libicule$(DLLPOST).$(ICU_MAJOR) \
    $(BUILD_DIR)$/lib$/libicule$(DLLPOST) \
    $(BUILD_DIR)$/lib$/libicutu$(DLLPOST).$(ICU_MAJOR).$(ICU_MINOR).$(ICU_MICRO) \
    $(BUILD_DIR)$/lib$/libicutu$(DLLPOST).$(ICU_MAJOR) \
    $(BUILD_DIR)$/lib$/libicutu$(DLLPOST)

OUT2BIN_NONE= \
    $(BUILD_DIR)$/bin$/genccode \
    $(BUILD_DIR)$/bin$/genbrk \
    $(BUILD_DIR)$/bin$/gencmn
.ENDIF

.ENDIF

.IF "$(GUI)"=="WNT"
CONFIGURE_DIR=source
.IF "$(COM)"=="GCC"
.IF "$(MINGW_SHARED_GCCLIB)"=="YES"
icu_LDFLAGS+=-shared-libgcc
.ENDIF
icu_LDFLAGS+=-L$(COMPATH)$/lib
icu_LIBS=
.IF "$(MINGW_SHARED_GXXLIB)"=="YES"
icu_LIBS+=$(MINGW_SHARED_LIBSTDCPP)
.ENDIF
icu_LDFLAGS+=-Wl,--enable-runtime-pseudo-reloc-v2

.IF "$(CROSS_COMPILING)"=="YES"
# We require that the cross-build-toolset target from the top Makefile(.in) has bee built
BUILD_AND_HOST=--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM) --with-cross-build=$(posix_PWD)/$(INPATH_FOR_BUILD)/misc/build/icu/source
.ELSE
BUILD_AND_HOST=--build=i586-pc-mingw32 --enable-64bit-libs=no
.ENDIF

CONFIGURE_ACTION+=sh -c 'CFLAGS="-O -D_MT" CXXFLAGS="-O -D_MT" LDFLAGS="$(icu_LDFLAGS)" LIBS="$(icu_LIBS)" \
./configure $(BUILD_AND_HOST) --enable-layout --disable-static --enable-shared --disable-samples'

CONFIGURE_FLAGS=

# Use of
# CONFIGURE_ACTION=sh -c 'CFLAGS=-O CXXFLAGS=-O ./configure'
# CONFIGURE_FLAGS=--enable-layout --enable-static --enable-shared=yes --enable-64bit-libs=no
# doesn't work as it would result in
# sh -c 'CFLAGS=-O CXXFLAGS=-O ./configure' --enable-layout ...
# note the position of the single quotes.

BUILD_DIR=$(CONFIGURE_DIR)
BUILD_ACTION=$(GNUMAKE) -j$(EXTMAXPROCESS)
OUT2LIB=

OUT2BIN= \
    $(BUILD_DIR)$/lib$/icudt$(ICU_MAJOR)$(ICU_MINOR)$(DLLPOST) \
    $(BUILD_DIR)$/lib$/icuuc$(ICU_MAJOR)$(ICU_MINOR)$(DLLPOST) \
    $(BUILD_DIR)$/lib$/icuin$(ICU_MAJOR)$(ICU_MINOR)$(DLLPOST) \
    $(BUILD_DIR)$/lib$/icule$(ICU_MAJOR)$(ICU_MINOR)$(DLLPOST) \
    $(BUILD_DIR)$/lib$/icutu$(ICU_MAJOR)$(ICU_MINOR)$(DLLPOST) \
    $(BUILD_DIR)$/bin$/genccode.exe \
    $(BUILD_DIR)$/bin$/genbrk.exe \
    $(BUILD_DIR)$/bin$/gencmn.exe

.ELSE
BUILD_DIR=source
CONFIGURE_ACTION=bash ./runConfigureICU Cygwin/MSVC
BUILD_ACTION=$(GNUMAKE) -j$(EXTMAXPROCESS)

OUT2LIB= \
    $(BUILD_DIR)$/lib$/icudt.lib \
    $(BUILD_DIR)$/lib$/icuin$(ICU_BUILD_LIBPOST).lib \
    $(BUILD_DIR)$/lib$/icuuc$(ICU_BUILD_LIBPOST).lib \
    $(BUILD_DIR)$/lib$/icule$(ICU_BUILD_LIBPOST).lib \
    $(BUILD_DIR)$/lib$/icutu$(ICU_BUILD_LIBPOST).lib

OUT2BIN= \
    $(BUILD_DIR)$/lib$/icudt$(ICU_MAJOR).dll \
    $(BUILD_DIR)$/lib$/icuin$(ICU_MAJOR)$(ICU_BUILD_LIBPOST).dll \
    $(BUILD_DIR)$/lib$/icuuc$(ICU_MAJOR)$(ICU_BUILD_LIBPOST).dll \
    $(BUILD_DIR)$/lib$/icule$(ICU_MAJOR)$(ICU_BUILD_LIBPOST).dll \
    $(BUILD_DIR)$/lib$/icutu$(ICU_MAJOR)$(ICU_BUILD_LIBPOST).dll \
    $(BUILD_DIR)$/bin$/genccode.exe \
    $(BUILD_DIR)$/bin$/genbrk.exe \
    $(BUILD_DIR)$/bin$/gencmn.exe

.ENDIF
.ENDIF		# "$(GUI)"=="WNT"

#make sure that when we deliver the headers of a new icu that the timestamps
#are newer than the last icu to ensure dependencies are correctly rebuilt
INSTALL_ACTION=find . -name "*.h" -print0 | xargs -0 touch

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk

.IF "$(BINARY_PATCH_FILES)"!=""

$(PACKAGE_DIR)$/so_add_binary :  $(PACKAGE_DIR)$/$(ADD_FILES_FLAG_FILE)
    cd $(PACKAGE_DIR) && gunzip -c $(BACK_PATH)$(BINARY_PATCH_FILES) | tar -xvf -
    $(TOUCH) $(PACKAGE_DIR)$/so_add_binary

$(PACKAGE_DIR)$/$(CONFIGURE_FLAG_FILE) : $(PACKAGE_DIR)$/so_add_binary

.ENDIF

.IF "$(GUI)$(COM)"=="WNTGCC"
ALLTAR : \
    $(LB)$/icudata.lib \
    $(LB)$/icuin$(ICU_BUILD_LIBPOST).lib \
    $(LB)$/icuuc$(ICU_BUILD_LIBPOST).lib \
    $(LB)$/icule$(ICU_BUILD_LIBPOST).lib \
    $(LB)$/icutu$(ICU_BUILD_LIBPOST).lib

$(LB)$/icudata.lib : $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE)
    $(TOUCH) $@

$(LB)$/icuin$(ICU_BUILD_LIBPOST).lib : $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE)
    $(TOUCH) $@

$(LB)$/icuuc$(ICU_BUILD_LIBPOST).lib : $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE)
    $(TOUCH) $@

$(LB)$/icule$(ICU_BUILD_LIBPOST).lib : $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE)
    $(TOUCH) $@

$(LB)$/icutu$(ICU_BUILD_LIBPOST).lib : $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE)
    $(TOUCH) $@
.ENDIF
