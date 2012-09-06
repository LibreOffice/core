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
TARFILE_MD5=aa346e934019d412ea6905563225f3cb

PATCH_FILES=

.IF "$(GUI)$(COM)"=="WNTMSC"

BUILD_DIR=vsprojects/liborcus-static-nozip
BUILD_ACTION= \
	export BOOST_INCLUDE_DIR=$(OUTDIR)/inc/external && \
	export BOOST_LIB_DIR=$(OUTDIR)/lib && \
	$(COMPATH)$/vcpackages$/vcbuild.exe liborcus-static-nozip.vcproj "Release|Win32"

.ELSE

CONFIGURE_DIR=
CONFIGURE_ACTION=./configure \
	--with-pic \
	--enable-static \
	--disable-shared \
	--without-libzip \
	--disable-debug \
	--disable-spreadsheet-model

BUILD_ACTION=make
BUILD_DIR=

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk

