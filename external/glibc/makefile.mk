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

PRJ=..

PRJNAME=external
TARGET=getopt

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

.IF "$(HAVE_GETOPT)" != "YES" || "$(HAVE_READDIR_R)" != "YES"
TARFILE_NAME=glibc-2.1.3-stub
TARFILE_MD5=4a660ce8466c9df01f19036435425c3a
TARFILE_ROOTDIR=glibc-2.1.3
ADDITIONAL_FILES=posix$/makefile.mk posix$/config.h
.IF "$(HAVE_READDIR_R)" != "YES"
ADDITIONAL_FILES += posix$/readdir_r.c
.ENDIF

PATCH_FILES=$(PRJ)$/glibc-2.1.3.patch

CONFIGURE_ACTION=

BUILD_DIR=posix
BUILD_ACTION=dmake $(MFLAGS) $(CALLMACROS)

OUT2INC= \
    posix/getopt.h \
    posix/config.h

.ELSE
@all:
    @echo "Nothing to do here."
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

