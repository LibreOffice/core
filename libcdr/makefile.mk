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

PRJNAME=visio
TARGET=visio

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

# --- Files --------------------------------------------------------

.IF "$(SYSTEM_LIBCDR)" == "YES"
@all:
    @echo "Using system libcdr..."
.ENDIF

# libcdr depends on the libwpd and libwpg
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

TARFILE_NAME=libcdr-0.0.1
TARFILE_MD5=dbf30c0dd92beee07d184738c242f073

PATCH_FILES=\
    libcdr-0.0.1.patch

BUILD_ACTION=dmake $(MFLAGS) $(CALLMACROS)
BUILD_DIR=src$/lib

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk
