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

PRJNAME=epm
TARGET=epm

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=epm-3.7
TARFILE_MD5=3ade8cfe7e59ca8e65052644fed9fca4
PATCH_FILES=epm-3.7.patch

.IF "$(GUI)"=="UNX" && "$(BUILD_EPM)" != "NO" && "$(CROSS_COMPILING)" != "YES"

EXTRPATH = NONE

CONFIGURE_ACTION=.$/configure
CONFIGURE_FLAGS=--disable-fltk
.IF "$(OS)"=="MACOSX"
CONFIGURE_FLAGS += \
    --prefix=/@.__________________________________________________$(EXTRPATH)
.IF "$(EXTRA_CFLAGS)"!=""
CONFIGURE_FLAGS+=CFLAGS="$(EXTRA_CFLAGS)" LDFLAGS="$(EXTRA_LINKFLAGS)" CPP="gcc -E $(EXTRA_CFLAGS)"
.ENDIF # "$(EXTRA_CFLAGS)"!=""
.ENDIF
BUILD_ACTION=make
OUT2BIN=epm epminstall mkepmlist

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

