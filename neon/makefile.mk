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

PRJNAME=so_neon
TARGET=so_neon

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

.IF "$(SYSTEM_NEON)" == "YES"
@all:
    @echo "Using system neon...."
.ENDIF

.IF "$(DISABLE_NEON)" == "TRUE"
@all:
    @echo "neon disabled...."
.ENDIF

NEON_NAME=neon-0.29.5

TARFILE_NAME=$(NEON_NAME)
TARFILE_MD5=ff369e69ef0f0143beb5626164e87ae2
PATCH_FILES=neon.patch

.IF "$(GUI)"=="WNT"
    PATCH_FILES+=neon_exports_win.patch
.ELSE
    PATCH_FILES+=neon_exports_unix.patch
.ENDIF

ADDITIONAL_FILES=src$/makefile.mk src$/config.h

BUILD_DIR=src
BUILD_ACTION=dmake $(MFLAGS) $(CALLMACROS)

OUT2INC= \
    $(BUILD_DIR)$/ne_207.h \
    $(BUILD_DIR)$/ne_alloc.h \
    $(BUILD_DIR)$/ne_auth.h \
    $(BUILD_DIR)$/ne_basic.h \
    $(BUILD_DIR)$/ne_compress.h \
    $(BUILD_DIR)$/ne_defs.h \
    $(BUILD_DIR)$/ne_locks.h \
    $(BUILD_DIR)$/ne_props.h \
    $(BUILD_DIR)$/ne_redirect.h \
    $(BUILD_DIR)$/ne_request.h \
    $(BUILD_DIR)$/ne_session.h \
    $(BUILD_DIR)$/ne_socket.h \
    $(BUILD_DIR)$/ne_ssl.h \
    $(BUILD_DIR)$/ne_string.h \
    $(BUILD_DIR)$/ne_uri.h \
    $(BUILD_DIR)$/ne_utils.h \
    $(BUILD_DIR)$/ne_xml.h \
    $(BUILD_DIR)$/ne_xmlreq.h

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

