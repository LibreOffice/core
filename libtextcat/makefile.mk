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

PRJNAME=libtextcat
TARGET=libtextcat

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=libtextcat-2.2
TARFILE_ROOTDIR=libtextcat-2.2

PATCH_FILE_NAME=libtextcat-2.2.patch


ADDITIONAL_FILES= \
                src$/utf8misc.h \
                src$/utf8misc.c \
                src$/win32_config.h \
                src$/makefile.mk \
                src$/libtextcat.map

.IF "$(GUI)"=="UNX"
#CONFIGURE_DIR=$(BUILD_DIR)

#relative to CONFIGURE_DIR
CONFIGURE_ACTION=configure CFLAGS="$(EXTRA_CFLAGS)"
CONFIGURE_FLAGS=

BUILD_ACTION=make

OUT2LIB=$(BUILD_DIR)$/src$/.libs$/libtextcat*$(DLLPOST)

.ENDIF # "$(GUI)"=="UNX"


.IF "$(GUI)"=="WNT" || "$(GUI)"=="OS2"
BUILD_ACTION=cd src && dmake $(MAKEMACROS)
.ENDIF # "$(GUI)"=="WNT" || "$(GUI)"=="OS2"


OUT2INC= \
    $(BUILD_DIR)$/src$/config.h \
    $(BUILD_DIR)$/src$/common.h \
    $(BUILD_DIR)$/src$/fingerprint.h \
    $(BUILD_DIR)$/src$/textcat.h \
    $(BUILD_DIR)$/src$/wg_mempool.h


# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

