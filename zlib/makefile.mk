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

PRJ=.

PRJNAME=zlib
TARGET=zlib

.IF "$(GUI)" == "UNX"
.IF "$(SYSTEM_ZLIB)" == "YES"
all:
    @echo "An already available installation of zlib should exist on your system."
    @echo "Therefore the version provided here does not need to be built in addition."
.ENDIF
.ENDIF

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=zlib-1.1.4

PATCH_FILE_NAME=zlib-1.1.4.patch
ADDITIONAL_FILES=makefile.mk

#relative to CONFIGURE_DIR

BUILD_DIR=$(CONFIGURE_DIR)
BUILD_ACTION=dmake $(MFLAGS) $(CALLMACROS)

OUT2INC= \
    zlib.h \
    zconf.h \
    contrib$/minizip$/unzip.h

PATCHED_HEADERS=$(INCCOM)$/patched$/zlib.h

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk

ALLTAR: $(PATCHED_HEADERS)

$(PATCHED_HEADERS) : $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE)
    @$(PERL) make_patched_header.pl $@ $(PRJNAME)
