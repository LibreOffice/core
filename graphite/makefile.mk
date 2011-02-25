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

# TODO: enable warnings again when external module compiles without warnings on all platforms
EXTERNAL_WARNINGS_NOT_ERRORS := TRUE

PRJ=.

PRJNAME=graphite
TARGET=so_graphite

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(SYSTEM_GRAPHITE)" == "YES"
all:
    @echo "An already available installation of graphite2 should exist on your system."
    @echo "Therefore the version provided here does not need to be built in addition."
.ENDIF

# --- Files --------------------------------------------------------
.IF "$(ENABLE_GRAPHITE)"=="TRUE"
TARFILE_NAME=graphite2-0.9.2
TARFILE_MD5=0625a7d661f899a8ce263fc8a9879108
PATCH_FILES= graphite2-0.9.2.patch

# convert line-endings to avoid problems when patching
CONVERTFILES=

CONFIGURE_DIR=build

.IF "$(COM)"=="MSC"
CMAKE_GENERATOR="NMake Makefiles"
# make use of stlport headerfiles
EXT_USE_STLPORT=TRUE
GR_CMAKE_LINK_FLAGS=-D "CMAKE_SHARED_LINKER_FLAGS=/STACK:10000000 /machine:x86" -D "CMAKE_MODULE_LINKER_FLAGS=/STACK:10000000 /machine:x86" -D "CMAKE_EXE_LINKER_FLAGS=/STACK:10000000 /machine:x86" -D "CMAKE_SYSTEM_PROCESSOR=x86"
BUILD_ACTION=nmake
.ENDIF

.IF "$(COM)"=="GCC"
CMAKE_GENERATOR="Unix Makefiles"
GR_CMAKE_LINK_FLAGS=
.IF "$(OS)"=="WNT"
#PATCH_FILES+=graphite2.patch.mingw
.ENDIF
.ENDIF

.IF "$(COM)"=="GCC"
BUILD_ACTION=$(GNUMAKE) -j$(EXTMAXPROCESS)
.ENDIF

.IF "$(debug)"=="true"
CMAKE_BUILD_TYPE=Debug
.ELSE
CMAKE_BUILD_TYPE=Release
.ENDIF

# Don't include STLPORT headers because it interferes with CMake's compiler
# detection. Graphite2 no longer uses the STL anyway.
CONFIGURE_ACTION=bash -c 'INCLUDE="$(COMPATH)/Include;$(PSDK_HOME)/Include" CXXFLAGS="$(CFLAGSCXX) $(CDEFS)" LIB="$(ILIB)" cmake -G $(CMAKE_GENERATOR) -D CMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) -D ENABLE_COMPARE_RENDERER=0 $(GR_CMAKE_LINK_FLAGS) .. '


BUILD_DIR=$(CONFIGURE_DIR)

.IF "$(OS)"=="WNT" && "$(COM)"!="GCC"
OUT2LIB=build$/src$/*.lib
OUT2BIN=build$/src$/*.dll
.ELSE
.IF "$(OS)"=="MACOSX"
OUT2LIB+=build$/src$/libgraphite2.*.dylib
.ELSE
OUT2LIB=build$/src$/libgraphite2.so.*.*.*
.ENDIF
.ENDIF

OUTDIR2INC= \
    include/graphite2

.ELSE
dddd:
    @echo Nothing to do
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :	set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk

