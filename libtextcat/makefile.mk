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

.IF "$(SYSTEM_LIBEXTTEXTCAT)" == "YES"
all:
        @echo "An already available installation of libtextcat should exist on your system."
        @echo "Therefore the version provided here does not need to be built in addition."
.ENDIF

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=libexttextcat-3.0.1
TARFILE_MD5=131b91de2d1df0ff5f0a8284b5417f8b
TARFILE_ROOTDIR=libexttextcat-3.0.1

.IF "$(GUI)"=="UNX"
#relative to CONFIGURE_DIR
CONFIGURE_ACTION=configure --disable-shared --with-pic CFLAGS="$(ARCH_FLAGS) $(EXTRA_CFLAGS)"
CONFIGURE_FLAGS=$(eq,$(OS),MACOSX CPPFLAGS="$(EXTRA_CDEFS)" $(NULL))
.IF "$(OS)"=="AIX"
CONFIGURE_FLAGS+= CFLAGS=-D_LINUX_SOURCE_COMPAT
.ENDIF
.IF "$(CROSS_COMPILING)"=="YES"
CONFIGURE_FLAGS+= --build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)
.ENDIF

BUILD_ACTION=make

OUT2LIB=$(BUILD_DIR)$/src$/.libs$/libtextcat.a

.ENDIF # "$(GUI)"=="UNX"


.IF "$(GUI)"=="WNT"
BUILD_ACTION=cd src && dmake $(MAKEMACROS)
.ENDIF # "$(GUI)"=="WNT"

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

