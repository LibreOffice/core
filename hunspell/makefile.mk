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
# $Revision: 1.4 $
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

PRJNAME=hunspell
TARGET=hunspell

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=hunspell-1.1.12-2
TARFILE_ROOTDIR=hunspell-1.1.12

#ADDITIONAL_FILES += src/hunspell/makefile.mk

PATCH_FILE_NAME=hunspell-1.1.12.patch

.IF "$(GUI)"=="UNX"
#CONFIGURE_DIR=$(BUILD_DIR)

#relative to CONFIGURE_DIR
CONFIGURE_ACTION=configure
CONFIGURE_FLAGS= --disable-shared --without-ui --without-readline --with-pic

BUILD_ACTION=make && make check

OUT2LIB=$(BUILD_DIR)$/src$/hunspell$/.libs$/libhunspell-1.1.a

.ENDIF # "$(GUI)"=="UNX"


.IF "$(GUI)"=="WNT"
BUILD_ACTION=cd src/hunspell && dmake
.ENDIF # "$(GUI)"=="WNT"


OUT2INC= \
    $(BUILD_DIR)$/src$/hunspell$/*.hxx

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

