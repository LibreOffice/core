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

PRJNAME=libcroco
TARGET=so_libcroco

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(SYSTEM_LIBCROCO)" == "YES"
all:
    @echo "An already available installation of libcroco should exist on your system."
    @echo "Therefore the version provided here does not need to be built in addition."
.ENDIF

# --- Files --------------------------------------------------------

LIBCROCOVERSION=0.6.2

TARFILE_NAME=$(PRJNAME)-$(LIBCROCOVERSION)
TARFILE_MD5=0611e099e807210cf738dcb41425d104

PATCH_FILES=
CONFIGURE_DIR=

.IF "$(OS)"=="MACOSX"
CONFIGURE_ACTION=$(AUGMENT_LIBRARY_PATH) \
                 .$/configure \
                 --prefix=$(SRC_ROOT)$/$(PRJNAME)$/$(MISC) \
                 CFLAGS="$(ARCH_FLAGS) $(EXTRA_CFLAGS) -I$(SOLARINCDIR)$/external -I$(SOLARINCDIR)$/external$/glib-2.0" \
                 LDFLAGS="-L$(SOLARLIBDIR)" \
                 GLIB2_CFLAGS="-I$(SOLARINCDIR)$/external$/glib-2.0" \
                 GLIB2_LIBS="-lgio-2.0 -lgobject-2.0 -lgthread-2.0 -lglib-2.0 -lintl" \
                 LIBXML2_CFLAGS=" " \
                 LIBXML2_LIBS="-lxml2"
                 
CONFIGURE_FLAGS=$(eq,$(OS),MACOSX CPPFLAGS="$(EXTRA_CDEFS)" $(NULL))
                
BUILD_ACTION=$(AUGMENT_LIBRARY_PATH) \
             $(GNUMAKE)
BUILD_DIR=$(CONFIGURE_DIR)

EXTRPATH=LOADER
OUT2LIB+=src$/.libs$/libcroco-0.6.3.0.1.dylib

OUT2INC+=src$/cr-additional-sel.h
OUT2INC+=src$/cr-input.h
OUT2INC+=src$/cr-rgb.h
OUT2INC+=src$/cr-stylesheet.h
OUT2INC+=src$/cr-attr-sel.h
OUT2INC+=src$/cr-num.h
OUT2INC+=src$/cr-sel-eng.h
OUT2INC+=src$/cr-term.h
OUT2INC+=src$/cr-cascade.h
OUT2INC+=src$/cr-om-parser.h
OUT2INC+=src$/cr-selector.h
OUT2INC+=src$/cr-tknzr.h
OUT2INC+=src$/cr-declaration.h
OUT2INC+=src$/cr-parser.h
OUT2INC+=src$/cr-simple-sel.h
OUT2INC+=src$/cr-token.h
OUT2INC+=src$/cr-doc-handler.h
OUT2INC+=src$/cr-parsing-location.h
OUT2INC+=src$/cr-statement.h
OUT2INC+=src$/cr-utils.h
OUT2INC+=src$/cr-enc-handler.h
OUT2INC+=src$/cr-prop-list.h
OUT2INC+=src$/cr-string.h
OUT2INC+=src$/libcroco-config.h
OUT2INC+=src$/cr-fonts.h
OUT2INC+=src$/cr-pseudo.h
OUT2INC+=src$/cr-style.h
OUT2INC+=src$/libcroco.h
.ELIF "$(OS)"=="WNT"

BUILD_ACTION=dmake
BUILD_DIR=$(CONFIGURE_DIR)$/src
PATCH_FILES=libcroco-0.6.2.patch
ADDITIONAL_FILES=\
    src$/makefile.mk

OUT2INC+=src$/cr-additional-sel.h
OUT2INC+=src$/cr-input.h
OUT2INC+=src$/cr-rgb.h
OUT2INC+=src$/cr-stylesheet.h
OUT2INC+=src$/cr-attr-sel.h
OUT2INC+=src$/cr-num.h
OUT2INC+=src$/cr-sel-eng.h
OUT2INC+=src$/cr-term.h
OUT2INC+=src$/cr-cascade.h
OUT2INC+=src$/cr-om-parser.h
OUT2INC+=src$/cr-selector.h
OUT2INC+=src$/cr-tknzr.h
OUT2INC+=src$/cr-declaration.h
OUT2INC+=src$/cr-parser.h
OUT2INC+=src$/cr-simple-sel.h
OUT2INC+=src$/cr-token.h
OUT2INC+=src$/cr-doc-handler.h
OUT2INC+=src$/cr-parsing-location.h
OUT2INC+=src$/cr-statement.h
OUT2INC+=src$/cr-utils.h
OUT2INC+=src$/cr-enc-handler.h
OUT2INC+=src$/cr-prop-list.h
OUT2INC+=src$/cr-string.h
OUT2INC+=src$/libcroco-config.h
OUT2INC+=src$/cr-fonts.h
OUT2INC+=src$/cr-pseudo.h
OUT2INC+=src$/cr-style.h
OUT2INC+=src$/libcroco.h
.ELSE

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

