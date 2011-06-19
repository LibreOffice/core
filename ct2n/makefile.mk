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
# $Revision: 1.10 $
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

PRJNAME=ct2n
TARGET=ct2n

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

TARFILE_MD5=451ccf439a36a568653b024534669971
TARFILE_NAME=ConvertTextToNumber-1.3.2
TARFILE_ROOTDIR=.
BUILD_DIR=.

PATCH_FILES=\
    ConvertTextToNumber-1.3.2-no-license.patch \
    ConvertTextToNumber-1.3.2-no-visible-by-default.patch

CONFIGURE_DIR=
CONFIGURE_ACTION=
CONFIGURE_FLAGS=
BUILD_ACTION=

ZIP1TARGET=ConvertTextToNumber
ZIP1EXT=.oxt
ZIP1FLAGS=-u -r
ZIP1DIR=$(MISC)/build$/$(TARFILE_ROOTDIR)
ZIP1LIST= * -x "*.orig" -x "so_*_ct2n"
ZIP1DEPS=$(PACKAGE_DIR)/$(PREDELIVER_FLAG_FILE)

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : tg_ext.mk
.INCLUDE : target.mk

ZIP1TARGET : $(PACKAGE_DIR)/$(PREDELIVER_FLAG_FILE)
