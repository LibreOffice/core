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
# $Revision: 1.8 $
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

# Hopefully we can move to a newer boost version soon, coming with spirit 1.8.*,
# so there won't be the need handle a separate spirit 1.6.* anymore.
# Depends on the compilers... :(

# dmake create_clean -- just unpacks
# dmake patch -- unpacks and applies patch file
# dmake create_patch -- creates a patch file

PRJ=..

PRJNAME=ooo_boost
TARGET=ooo_spirit

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# force patched boost for sunpro CC
# to workaround opt bug when compiling with -xO3
.IF "$(SYSTEM_BOOST)" == "YES" && ("$(OS)"!="SOLARIS" || "$(COM)"=="GCC")
all:
        @echo "An already available installation of boost should exist on your system."
        @echo "Therefore the version provided here does not need to be built in addition."
.ELSE

# --- Files --------------------------------------------------------

.IF "$(COMID)"=="gcc3" && "$(CCNUMVER)">="000400000000" 
all:
        @echo "spirit is already included in boost 1.34"
.ELSE

TARFILE_NAME=spirit-1.6.1
PATCH_FILE_NAME=$(TARFILE_NAME).patch

CONFIGURE_DIR=
CONFIGURE_ACTION=

BUILD_DIR=
BUILD_ACTION=
BUILD_FLAGS=

# --- Targets ------------------------------------------------------

all: \
    $(MISC)$/$(TARGET)_remove_build.flag \
    ALLTAR

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

# Since you never know what will be in a patch (for example, it may already
# patch at configure level), we remove the entire package directory if a patch
# is newer.
$(MISC)$/$(TARGET)_remove_build.flag : $(PRJ)$/$(PATCH_FILE_NAME)
    $(REMOVE_PACKAGE_COMMAND)
    $(TOUCH) $(MISC)$/$(TARGET)_remove_build.flag

.ENDIF
.ENDIF
