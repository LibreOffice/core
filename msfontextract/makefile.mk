#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.7 $
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

PRJNAME=msfontextract
TARGET=libmspack
NO_DEFAULT_STL=TRUE
LIBSALCPPRT=

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

.IF "$(WITH_FONTOOO)" != "YES"
@all:
    @echo "FontOOo disabled... Nothing to build here."
.ENDIF

TARFILE_NAME=libmspack
PATCH_FILE_NAME=msfontextract.patch

ADDITIONAL_FILES=mspack$/makefile.mk mspack$/msfontextract.c mspack$/ministub.c

CONFIGURE_ACTION=none
CONFIGURE_FLAGS=

BUILD_DIR=mspack
BUILD_FLAGS=
BUILD_ACTION=dmake subdmak=true $(MFLAGS) $(MAKEFILE) $(CALLMACROS)

.INCLUDE : 	set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk

