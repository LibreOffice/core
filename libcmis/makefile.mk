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

PRJNAME=cmis
TARGET=cmis

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

.IF "$(SYSTEM_LIBCMIS)" == "YES"
@all:
    @echo "Using system libcmis..."
.ENDIF

TARFILE_NAME=libcmis-0.1.0
TARFILE_MD5=7c2549f6b0a8bb604e6c4c729ffdcfe6
ADDITIONAL_FILES=src$/libcmis$/makefile.mk

PATCH_FILES=$(TARFILE_NAME).patch

BUILD_ACTION=dmake $(MFLAGS) $(CALLMACROS)
BUILD_DIR=src$/libcmis

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk

