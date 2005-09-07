#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 20:01:10 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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

NEON_NAME=neon-0.24.7

TARFILE_NAME=$(NEON_NAME)
PATCH_FILE_NAME=neon.patch

ADDITIONAL_FILES=src$/makefile.mk src$/config.h

BUILD_DIR=src
BUILD_ACTION=dmake $(MFLAGS) $(CALLMACROS)

OUT2INC= \
    $(BUILD_DIR)$/config.h \
    $(BUILD_DIR)$/ne_207.h \
    $(BUILD_DIR)$/ne_alloc.h \
    $(BUILD_DIR)$/ne_auth.h \
    $(BUILD_DIR)$/ne_basic.h \
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
    $(BUILD_DIR)$/ne_xml.h

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

