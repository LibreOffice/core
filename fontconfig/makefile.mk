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

PRJNAME=fontconfig
TARGET=fontconfig

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=fontconfig-2.8.0
TARFILE_MD5=77e15a92006ddc2adbb06f840d591c0e

PATCH_FILES=$(TARFILE_NAME).patch

CONFIGURE_ACTION=./configure

.IF "$(debug)" != ""
CONFIGURE_ACTION!:=CFLAGS=-g $(CONFIGURE_ACTION)
.ENDIF

# This "bundled" fontconfig is built only when cross-compiling for Android

CONFIGURE_FLAGS=\
    --disable-shared \
    --with-arch=arm \
    --with-expat-includes=$(SOLARVER)/$(INPATH)/inc/external \
    --with-expat-lib=$(SOLARVER)/$(INPATH)/lib \
    --with-freetype-config=$(SOLARVER)/$(INPATH)/bin/freetype-config \
    --build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)

BUILD_ACTION=$(GNUMAKE)

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

