#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

PRJ=.

PRJNAME=orcus
TARGET=orcus

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

.IF "$(SYSTEM_LIBORCUS)" == "YES"
@all:
    @echo "Using system liborcus..."
.ENDIF

TARFILE_NAME=liborcus_0.1.0
TARFILE_MD5=46d9f4cf8b145c21ce1056e116d2ce71

PATCH_FILES=liborcus_0.1.0-warnings.patch
    # -Werror,-Wunused-variable -Werror,-Wunused-private-field

PATCH_FILES+=liborcus_0.1.0-configure.patch
    # make config.sub recognize arm-linux-androideabi

# fix MinGW build
PATCH_FILES+=liborcus_0.1.0-mingw.patch

.IF "$(GUI)$(COM)"=="WNTMSC"

BUILD_DIR=vsprojects/liborcus-static-nozip
BUILD_ACTION= \
	export BOOST_INCLUDE_DIR=$(OUTDIR)/inc/external && \
	export BOOST_LIB_DIR=$(OUTDIR)/lib && \
	$(COMPATH)$/vcpackages$/vcbuild.exe liborcus-static-nozip.vcproj "Release|Win32"

.ELSE

.IF "$(SYSTEM_BOOST)" == "NO"
MY_CXXFLAGS = CXXFLAGS=-I$(OUTDIR)/inc/external
.END

CONFIGURE_DIR=
CONFIGURE_ACTION=./configure \
	--with-pic \
	--enable-static \
	--disable-shared \
	--without-libzip \
	--disable-debug \
	--disable-spreadsheet-model $(MY_CXXFLAGS)

.IF "$(CROSS_COMPILING)" == "YES"
CONFIGURE_ACTION+=--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)
.ENDIF

.IF "$(OS)" == "ANDROID"
CONFIGURE_ACTION:=LIBS='-lgnustl_shared -lm' $(CONFIGURE_ACTION)
.ENDIF

BUILD_ACTION=make
BUILD_DIR=

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk

