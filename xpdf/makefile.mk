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
# $Revision: 1.7.4.1 $
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

.IF "$(SYSTEM_POPPLER)" == "YES"
dummy:
        @echo "An already available installation of poppler should exist on your system."
        @echo "Therefore xpdf provided here does not need to be built in addition."
.ENDIF

# --- Files --------------------------------------------------------

TARFILE_NAME=xpdf-3.02
PATCH_FILES=$(TARFILE_NAME).patch

CONFIGURE_DIR=
BUILD_DIR=$(CONFIGURE_DIR)

.IF "$(SYSBASE)"!=""
.IF "$(EXTRA_CFLAGS)"!=""
CFLAGS:=$(EXTRA_CFLAGS)
CXXFLAGS:=$(EXTRA_CFLAGS)
.EXPORT : CFLAGS CXXFLAGS
.ENDIF # "$(EXTRA_CFLAGS)"!=""
.ENDIF # "$(SYSBASE)"!=""

.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="SOLARIS"
CFLAGS:=-O $(ARCH_FLAGS)
.IF "$(COM)$(CPU)" == "C52I"
CXXFLAGS:=-O2 $(ARCH_FLAGS)
.ELSE
CXXFLAGS:=-O $(ARCH_FLAGS)
.ENDIF
LDFLAGS:=$(ARCH_FLAGS)
.EXPORT : CFLAGS CXXFLAGS LDFLAGS
.ENDIF

.IF "$(COM)$(OS)$(CPU)" == "GCCMACOSXP"
CONFIGURE_ACTION=./configure --without-x --enable-multithreaded --enable-exceptions CXXFLAGS="-malign-natural"
.ELSE
#CONFIGURE_ACTION=./configure
#CONFIGURE_ACTION=./configure --without-x --enable-multithreaded --enable-exceptions CFLAGS="-g -O0" CXXFLAGS="-g -O0"
CONFIGURE_ACTION=./configure --without-libpaper-library --without-t1-library --without-x --enable-multithreaded --enable-exceptions
.ENDIF

BUILD_ACTION=$(GNUMAKE) -j$(EXTMAXPROCESS)
.ELSE
.IF "$(COM)"=="GCC"
CONFIGURE_ACTION=./configure --without-x --enable-multithreaded --enable-exceptions LIBS=-lgdi32
BUILD_ACTION=$(GNUMAKE) -j$(EXTMAXPROCESS)
.ELSE
CONFIGURE_ACTION=
BUILD_ACTION= cmd.exe /c ms_make.bat
.ENDIF
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
.IF "$(COM)"=="GCC"
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
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk
