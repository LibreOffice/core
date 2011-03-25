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

PRJNAME=so_freetype
TARGET=so_freetype

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(SYSTEM_FREETYPE)" == "YES"
all:
        @echo "An already available installation of freetype should exist on your system."
    @echo "Therefore the version provided here does not need to be built in addition."
.ENDIF

# --- Files --------------------------------------------------------

TARFILE_NAME=freetype-2.4.4
TARFILE_MD5=9273efacffb683483e58a9e113efae9f

.IF "$(OS)"=="WNT"

CONFIGURE_DIR=
.IF "$(OS)"=="WNT"
CONFIGURE_ACTION=
ADDITIONAL_FILES=config.mk \
            objs/ftmodule.h \
            freetype.def
PATCH_FILES=freetype-2.4.4.patch
LIBS= -l $(STDLIBGUIMT) -l $(LIBSTLPORT)
.ELSE
CONFIGURE_ACTION=.$/configure
.ENDIF

BUILD_ACTION=$(GNUMAKE)

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk
