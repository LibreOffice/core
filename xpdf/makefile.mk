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
# $Revision: 1.3 $
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

# dmake create_clean -- just unpacks
# dmake patch -- unpacks and applies patch file
# dmake create_patch -- creates a patch file

PRJ=.

PRJNAME=xpdf
TARGET=xpdflib

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=xpdf-3.02
PATCH_FILE_NAME=$(TARFILE_NAME).patch

CONFIGURE_DIR=
BUILD_DIR=$(CONFIGURE_DIR)

.IF "$(GUI)"=="UNX"
#CONFIGURE_ACTION=./configure
#CONFIGURE_ACTION=./configure --without-x --enable-multithreaded --enable-exceptions CFLAGS="-g -O0" CXXFLAGS="-g -O0"
CONFIGURE_ACTION=./configure --without-x --enable-multithreaded --enable-exceptions
BUILD_ACTION=$(GNUMAKE) -j$(EXTMAXPROCESS)
.ELSE
CONFIGURE_ACTION=
BUILD_ACTION= cmd.exe /c ms_make.bat
.ENDIF

CONVERTFILES=ms_make.bat
BUILD_FLAGS=

OUT2INC= \
    fofi$/*.h \
    goo$/*.h  \
    xpdf$/*.h \
    aconf.h   \
    aconf2.h

.IF "$(GUI)"=="UNX"
OUT2LIB= \
    fofi$/lib*.a \
    goo$/lib*.a \
    xpdf$/lib*.a
.ELSE
OUT2LIB= \
    fofi$/*.lib \
    goo$/*.lib \
    xpdf$/*.lib
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk
