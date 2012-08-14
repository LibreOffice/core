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

PRJNAME=hunspell
TARGET=hunspell

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=hunspell-1.3.2
TARFILE_MD5=3121aaf3e13e5d88dfff13fb4a5f1ab8
ADDITIONAL_FILES+=config.h

# hunspell-1.3.2-nullptr.patch, hunspell-1.3.2-literal.patch: reported upstream
#  as <https://sourceforge.net/tracker/?func=detail&aid=3557292&group_id=143754&
#  atid=756395> "Some C++11 related fixes"
PATCH_FILES=\
    hunspell-twoaffixcompound.patch \
    hunspell-static.patch \
    hunspell-wntconfig.patch \
    hunspell-solaris.patch \
    hunspell-1.3.2-overflow.patch \
    hunspell-android.patch \
    hunspell-1.3.2-nullptr.patch \
    hunspell-1.3.2-literal.patch

.IF "$(GUI)"=="UNX"

#relative to CONFIGURE_DIR
CONFIGURE_ACTION=$(AUGMENT_LIBRARY_PATH) configure
CONFIGURE_FLAGS= --disable-shared --disable-nls --with-pic
.IF "$(COMNAME)"=="sunpro5"
CONFIGURE_FLAGS+= CFLAGS=-xc99=none
.ENDIF                  # "$(COMNAME)"=="sunpro5"

.IF "$(OS)"=="AIX"
CONFIGURE_FLAGS+= CFLAGS=-D_LINUX_SOURCE_COMPAT
.ELIF "$(OS)" == "MACOSX"
CONFIGURE_FLAGS += \
    --prefix=/@.__________________________________________________$(EXTRPATH)
.END

.IF "$(SYSBASE)"!=""
.IF "$(EXTRA_CFLAGS)"!=""
CONFIGURE_FLAGS+= CFLAGS="$(EXTRA_CFLAGS)" CXXFLAGS="$(EXTRA_CFLAGS)"
.ENDIF # "$(EXTRA_CFLAGS)"!=""
.ELIF "$(OS)"=="MACOSX" # "$(SYSBASE)"!=""
CONFIGURE_FLAGS+=CPPFLAGS="$(EXTRA_CDEFS)"
.ENDIF

.IF "$(CROSS_COMPILING)"=="YES"
CONFIGURE_FLAGS+=--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM) LIBS="$(STDLIBCUIMT)"
.ENDIF

BUILD_ACTION=$(GNUMAKE) -j$(EXTMAXPROCESS)

OUT2LIB=$(BUILD_DIR)$/src$/hunspell$/.libs$/libhunspell-1.3.a

.ENDIF # "$(GUI)"=="UNX"


.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
CONFIGURE_ACTION=configure

CONFIGURE_FLAGS= --disable-shared --disable-nls --with-pic LDFLAGS=-Wl,--enable-runtime-pseudo-reloc-v2

.IF "$(CROSS_COMPILING)"=="YES"
CONFIGURE_FLAGS+=--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)
.ENDIF

BUILD_ACTION=make
OUT2LIB=$(BUILD_DIR)$/src$/hunspell$/.libs$/libhunspell-1.3.a
.ELSE
BUILD_ACTION=cd src/hunspell && dmake
.ENDIF
.ENDIF # "$(GUI)"=="WNT"

OUT2INC= \
    $(BUILD_DIR)$/src$/hunspell$/*.hxx \
    $(BUILD_DIR)$/src$/hunspell$/*.h

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

