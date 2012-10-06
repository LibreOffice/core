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

# TODO: enable warnings again when external module compiles without warnings on all platforms
EXTERNAL_WARNINGS_NOT_ERRORS := TRUE

PRJ=.

PRJNAME=graphite
TARGET=libgraphite

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(SYSTEM_GRAPHITE)" == "YES"
all:
	@echo "An already available installation of graphite2 should exist on your system."
	@echo "Therefore the version provided here does not need to be built in addition."
.ENDIF

# --- Files --------------------------------------------------------
.IF "$(ENABLE_GRAPHITE)"=="TRUE"
TARFILE_NAME=graphite2-1.2.0
TARFILE_MD5=f5ef3f7f10fa8c3542c6a085a233080b
PATCH_FILES= graphite2.patch \
        graphite_make.patch 

ADDITIONAL_FILES= \
    src/makefile.mk

# convert line-endings to avoid problems when patching
CONVERTFILES=

BUILD_DIR=src
BUILD_ACTION=dmake $(MFLAGS) $(CALLMACROS)

OUT2INC_SUBDIR=graphite2
OUT2INC=include/graphite2/Font.h \
        include/graphite2/Segment.h \
        include/graphite2/Types.h
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :	set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk

