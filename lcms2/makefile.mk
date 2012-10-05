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

PRJNAME=lcms2
TARGET=so_lcms2

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=lcms2-2.3
TARFILE_MD5=327348d67c979c88c2dec59a23a17d85

PATCH_FILES = lcms2.patch

.IF "$(SYSTEM_LCMS2)" == "YES"
@all:
    @echo "Using system littlecms2..."
.ENDIF

.IF "$(GUI)$(COM)"=="WNTMSC"

PATCH_FILES += lcms2-windows-export.patch

CONFIGURE_DIR=.

CONFIGURE_ACTION =
BUILD_DIR=Projects/VC2010/lcms2_DLL

.IF "$(CCNUMVER)" >= "001600000000"
BUILD_ACTION=MSBuild.exe lcms2_DLL.vcxproj /p:Configuration=Release /p:Platform=Win32 /p:TargetName=lcms2
.ELSE
BUILD_ACTION=$(COMPATH)$/vcpackages$/vcbuild.exe lcms2_DLL.vcproj "Release|Win32"
.ENDIF

.ELSE #"$(GUI)$(COM)"!="WNTMSC"

CONFIGURE_DIR=.
BUILD_DIR=src

CONFIGURE_ACTION = CPPFLAGS="$(SOLARINC)" ./configure --without-jpeg --without-tiff --with-pic --enable-shared --disable-static

.IF "$(CROSS_COMPILING)" == "YES"
CONFIGURE_ACTION += --build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)
.ENDIF

.IF "$(OS)" == "MACOSX"
CONFIGURE_FLAGS += \
    --prefix=/@.__________________________________________________$(EXTRPATH)
.END

.IF "$(OS)" == "IOS" || "$(OS)" == "ANDROID" 
CONFIGURE_ACTION += --disable-shared
.ENDIF

BUILD_ACTION = $(GNUMAKE) -j$(GMAKE_MODULE_PARALLELISM)

.ENDIF # "$(GUI)$(COM)"=="WNTMSC"

OUT2INC+=include$/lcms2*.h

.IF "$(OS)"=="MACOSX"
OUT2LIB+=src$/.libs$/liblcms2.*.dylib
.ELIF "$(OS)"=="IOS" || "$(OS)" == "ANDROID" 
OUT2LIB+=src$/.libs$/liblcms2.a
.ELIF "$(OS)"=="WNT"
.IF "$(COM)"=="GCC"
OUT2LIB+=src$/.libs$/liblcms2.dll.a
OUT2BIN+=src$/.libs$/*.dll
.ELSE
OUT2LIB+=bin$/lcms2.lib
OUT2BIN+=bin$/lcms2.dll
.ENDIF
.ELSE
OUT2LIB+=src$/.libs$/liblcms2.so*
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

