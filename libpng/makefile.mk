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

PRJNAME=libpng
TARGET=libpng

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(SYSTEM_LIBPNG)" == "YES"
all:
    @echo "An already available installation of libpng should exist on your system."
    @echo "Therefore the version provided here does not need to be built in addition."
.ENDIF

# --- Files --------------------------------------------------------

LIBPNGVERSION=1.5.1

TARFILE_NAME=$(PRJNAME)-$(LIBPNGVERSION)
TARFILE_MD5=220035f111ea045a51e290906025e8b5

PATCH_FILES=libpng.patch
ADDITIONAL_FILES=makefile.mk pnglibconf.h

#relative to CONFIGURE_DIR

BUILD_DIR=$(CONFIGURE_DIR)
BUILD_ACTION=dmake $(MFLAGS) $(CALLMACROS)

OUT2INC=png.h pnglibconf.h pngconf.h pngstruct.h
# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

