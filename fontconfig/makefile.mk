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

PRJNAME=so_fontconfig
TARGET=so_fontconfig

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(SYSTEM_FONTCONFIG)" == "YES"
all:
        @echo "An already available installation of freetype should exist on your system."
    @echo "Therefore the version provided here does not need to be built in addition."
.ENDIF

# --- Files --------------------------------------------------------

TARFILE_NAME=fontconfig-2.8.0
TARFILE_MD5=77e15a92006ddc2adbb06f840d591c0e

CONFIGURE_DIR=
.IF "$(OS)"=="WNT"
ADDITIONAL_FILES=\
            fc-arch$/fcalias.h \
            fc-arch$/fcaliastail.h \
            fc-arch$/fcarch.h \
            fc-glyphname$/fcglyphname.h \
            fc-lang$/fclang.h \
            src$/fcalias.h \
            src$/fcaliastail.h \
            src$/fcftaliastail.h \
            src$/makefile.mk \
            config.h

PATCH_FILES=fontconfig-2.8.0.patch
BUILD_DIR=$(CONFIGURE_DIR)$/src
BUILD_ACTION=dmake $(MFLAGS) $(CALLMACROS)
.ELSE
CONFIGURE_ACTION=./configure --enable-shared=yes --enable-libxml2 --disable-docs --libdir=$(OUTDIR)/lib
BUILD_ACTION=$(GNUMAKE)
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk
