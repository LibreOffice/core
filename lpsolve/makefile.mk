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

PRJNAME=lpsolve
TARGET=lpsolve

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=$(PRJNAME)-5.5.2.0
TARFILE_MD5=b7aeff93c2cf713056b30f832843ba3e

CONFIGURE_DIR=
CONFIGURE_ACTION=.$/configure
CONFIGURE_FLAGS=
.IF "$(CROSS_COMPILING)"=="YES"
BUILD_AND_HOST=--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)
CONFIGURE_FLAGS+=$(BUILD_AND_HOST)
.ENDIF # "$(CROSS_COMPILING)"=="YES"

.IF "$(OS)"=="IOS"
CONFIGURE_FLAGS+=--disable-shared
.ELSE # "$(OS)"=="IOS"
CONFIGURE_FLAGS+=--disable-static
.ENDIF # "$(OS)"=="IOS"

BUILD_DIR=$(CONFIGURE_DIR)
BUILD_ACTION=$(GNUMAKE) -j$(EXTMAXPROCESS)

OUTDIR2INC=.$/

.IF "$(OS)"=="MACOSX"
EXTRPATH=URELIB
OUT2LIB+=.libs$/liblpsolve55.*.dylib
.ELIF "$(OS)"=="IOS"
OUT2LIB+=.libs$/liblpsolve55.a
.ELIF "$(OS)"=="WNT"
.IF "$(COM)"=="GCC"
OUT2LIB+=.libs$/liblpsolve55*.a
.ELSE
OUT2LIB+=win32$/bin.msvc$/*.lib
.ENDIF
.ELSE
OUT2LIB+=.libs$/liblpsolve55.so*
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :	set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk

