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

TARFILE_NAME=libcmis-0.2.3
TARFILE_MD5=0d2dcdfbf28d6208751b33057f5361f0

# Pushed upstream in both master and libcmis-0.2 branches
PATCH_FILES+=libcmis-0.2.3.patch
# Pushed upstream to master branch, but not to libcmis-0.2
PATCH_FILES+=libcmis-0.2.3-allowable-actions.patch

.IF "$(OS)$(COM)" == "WNTMSC"
PATCH_FILES+=boost-win.patch
.ENDIF

.IF "$(debug)" != ""
CFLAGS+=-g
.ELSE
CFLAGS+=-O
.ENDIF

BUILD_ACTION=dmake $(MFLAGS) $(CALLMACROS)
BUILD_DIR=src$/libcmis

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk

