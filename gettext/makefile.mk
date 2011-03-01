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

PRJNAME=gettext
TARGET=so_gettext

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(SYSTEM_GETTEXT)" == "YES"
all:
    @echo "An already available installation of gettext should exist on your system."
    @echo "Therefore the version provided here does not need to be built in addition."
.ENDIF

# --- Files --------------------------------------------------------

GETTEXTVERSION=0.18.1.1

TARFILE_NAME=$(PRJNAME)-$(GETTEXTVERSION)
TARFILE_MD5=3dd55b952826d2b32f51308f2f91aa89

PATCH_FILES=

CONFIGURE_DIR=
CONFIGURE_ACTION=.$/configure --prefix=$(SRC_ROOT)$/$(PRJNAME)$/$(MISC) CFLAGS="$(ARCH_FLAGS) $(EXTRA_CFLAGS)"
CONFIGURE_FLAGS=$(eq,$(OS),MACOSX CPPFLAGS="$(EXTRA_CDEFS)" $(NULL)) --disable-curses --without-emacs --without-git --disable-java

BUILD_ACTION=$(GNUMAKE)
BUILD_DIR=$(CONFIGURE_DIR)

.IF "$(OS)"=="MACOSX"
EXTRPATH=LOADER
OUT2LIB+=gettext-tools$/intl$/.libs$/libintl.*.dylib
OUT2LIB+=gettext-runtime$/libasprintf$/.libs$/libasprintf.*.dylib
OUT2LIB+=gettext-tools$/gnulib-lib$/.libs$/libgettextlib-*.dylib
OUT2LIB+=gettext-tools$/src$/.libs$/libgettextsrc-*.dylib
OUT2LIB+=gettext-tools$/libgettextpo$/.libs$/libgettextpo.*.dylib

OUT2INC+=gettext-tools$/intl$/libintl.h
OUT2INC+=gettext-runtime$/libasprintf$/autosprintf.h
OUT2INC+=gettext-tools$/libgettextpo$/gettext-po.h

OUT2BIN+=gettext-tools$/src$/.libs$/msgfmt
.ELIF "$(OS)"=="WNT"
.ELSE
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

