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
TARFILE_MD5=599dc4cc65a07ee868cf92a667a913d2
PATCH_FILES=$(TARFILE_NAME).patch xpdf-3.02-ooopwd.patch xpdf-3.02-gfxColorMaxComps.patch xpdf-no-writable-literals.patch

CONFIGURE_DIR=
BUILD_DIR=$(CONFIGURE_DIR)

.IF "$(SYSBASE)"!=""
.IF "$(EXTRA_CFLAGS)"!=""
CFLAGS:=$(EXTRA_CFLAGS)
CXXFLAGS:=$(EXTRA_CFLAGS)
.EXPORT : CFLAGS CXXFLAGS
.ENDIF # "$(EXTRA_CFLAGS)"!=""
.ELIF "$(OS)"=="MACOSX" # "$(SYSBASE)"!=""
CFLAGS:=$(EXTRA_CDEFS)
CXXFLAGS+:=$(EXTRA_CDEFS)
.EXPORT: CFLAGS CXXFLAGS
.ENDIF

.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="SOLARIS"
CFLAGS:=-O $(ARCH_FLAGS)
.IF "$(COM)$(CPU)" == "C52I"
CXXFLAGS:=-O2 $(ARCH_FLAGS)
.ELSE
CXXFLAGS:=-O $(ARCH_FLAGS)
.ENDIF
.IF "$(SYSBASE)" != ""
CXXFLAGS +:= -I$(SYSBASE)/usr/include
.END
LDFLAGS:=$(ARCH_FLAGS)
.EXPORT : CFLAGS CXXFLAGS LDFLAGS
.ENDIF

CONFIGURE_ACTION=configure
CONFIGURE_FLAGS+=--without-x --without-libpaper-library --without-t1-library --enable-multithreaded --enable-exceptions

.IF "$(OS)" == "MACOSX"
CONFIGURE_FLAGS += \
    --prefix=/@.__________________________________________________$(EXTRPATH)
.END

.IF "$(OS)$(CPU)"=="MACOSXP"
CXXFLAGS+=-malign-natural
.EXPORT: CXXFLAGS
.ENDIF

BUILD_ACTION=$(GNUMAKE) -j$(EXTMAXPROCESS)
.ELSE
.IF "$(COM)"=="GCC"
LDFLAGS=-Wl,--enable-runtime-pseudo-reloc-v2
.EXPORT : LDFLAGS

CONFIGURE_ACTION=./configure
CONFIGURE_FLAGS+=--without-x --enable-multithreaded --enable-exceptions LIBS=-lgdi32

.IF "$(CROSS_COMPILING)"=="YES"
CONFIGURE_FLAGS+=--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)
.ENDIF

BUILD_ACTION=$(GNUMAKE) -j$(EXTMAXPROCESS)
.ELSE
CONFIGURE_ACTION=
BUILD_ACTION= cmd.exe /d /c ms_make.bat
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
