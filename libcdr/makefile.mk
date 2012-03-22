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

PRJNAME=cdr
TARGET=cdr

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

# --- Files --------------------------------------------------------

.IF "$(SYSTEM_LIBCDR)" == "YES"
@all:
    @echo "Using system libcdr..."
.ENDIF

# libcdr depends on the libwpd, libwpg and lcms2
.IF "$(SYSTEM_LIBWPD)" == "YES"
INCPRE+=$(WPD_CFLAGS)
.ELSE
INCPRE+=$(SOLARVER)$/$(INPATH)$/inc$/libwpd
INCPRE+=$(SOLARVER)$/$(INPATH)$/inc$/libwpd-stream
.ENDIF
.IF "$(SYSTEM_LIBWPG)" == "YES"
INCPRE+=$(WPG_CFLAGS)
.ELSE
INCPRE+=$(SOLARVER)$/$(INPATH)$/inc$/libwpg
.ENDIF
.IF "$(SYSTEM_LCMS2)" == "YES"
INCPRE+=$(LCMS2_CFLAGS)
.ELSE
INCPRE+=$(SOLARVER)$/$(INPATH)$/inc$/lcms2
.ENDIF

TARFILE_NAME=libcdr-0.0.5
TARFILE_MD5=4def42cfe3527ed7c515bea8cc3f23b8

PATCH_FILES=\
    $(TARFILE_NAME).patch \
    $(TARFILE_NAME)-zip.patch

BUILD_ACTION=dmake $(MFLAGS) $(CALLMACROS)
BUILD_DIR=src$/lib

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk
