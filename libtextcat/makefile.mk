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

PRJNAME=libtextcat
TARGET=libtextcat

.IF "$(SYSTEM_LIBTEXTCAT)" == "YES"
all:
        @echo "An already available installation of libtextcat should exist on your system."
        @echo "Therefore the version provided here does not need to be built in addition."
.ENDIF

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=libtextcat-2.2
TARFILE_MD5=128cfc86ed5953e57fe0f5ae98b62c2e
TARFILE_ROOTDIR=libtextcat-2.2

PATCH_FILES=\
    libtextcat-2.2.patch \
    libtextcat-aix.patch

ADDITIONAL_FILES= \
                src$/utf8misc.h \
                src$/utf8misc.c \
                src$/win32_config.h \
                src$/makefile.mk \
                src$/libtextcat.map

.IF "$(GUI)"=="UNX"
#relative to CONFIGURE_DIR
CONFIGURE_ACTION=configure CFLAGS="$(ARCH_FLAGS) $(EXTRA_CFLAGS)"
CONFIGURE_FLAGS=$(eq,$(OS),MACOSX CPPFLAGS="$(EXTRA_CDEFS)" $(NULL))
.IF "$(OS)"=="AIX"
CONFIGURE_FLAGS+= CFLAGS=-D_LINUX_SOURCE_COMPAT
.ENDIF

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

