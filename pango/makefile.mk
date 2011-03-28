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

PRJNAME=pango
TARGET=so_pango

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(SYSTEM_PANGO)" == "YES"
all:
    @echo "An already available installation of pango should exist on your system."
    @echo "Therefore the version provided here does not need to be built in addition."
.ENDIF

# --- Files --------------------------------------------------------

PANGOVERSION=1.28.3

TARFILE_NAME=$(PRJNAME)-$(PANGOVERSION)
TARFILE_MD5=22ad1c8d3fda7e73b0798035f3dd96bc

.IF "$(OS)"=="MACOSX" || "$(OS)"=="WNT"

.IF "$(OS)" == "MACOSX"

PATCH_FILES=pango-1.28.3.patch
ADDITIONAL_FILES=pango/glib-mkenums

CONFIGURE_LDFLAGS="-L$(SOLARLIBDIR)"
CONFIGURE_DIR=
CONFIGURE_ACTION=$(AUGMENT_LIBRARY_PATH) \
                 .$/configure --prefix=$(SRC_ROOT)$/$(PRJNAME)$/$(MISC) \
                 CFLAGS="$(ARCH_FLAGS) $(EXTRA_CFLAGS) $(EXTRA_CDEFS) -I$(SOLARINCDIR) -I$(SOLARINCDIR)$/external -I$(SOLARINCDIR)$/external$/glib-2.0" \
                 LDFLAGS="$(CONFIGURE_LDFLAGS)" \
                 CAIRO_CFLAGS="-I$(SOLARINCDIR) -I$(SOLARINCDIR)$/cairo" \
                 CAIRO_LIBS="-lcairo" \
                 GLIB_CFLAGS="-I$(SOLARINCDIR)$/external$/glib-2.0" \
                 GLIB_LIBS="-lgthread-2.0 -lgmodule-2.0 -lgobject-2.0 -lglib-2.0 -lintl"
CONFIGURE_FLAGS=--with-included-modules=yes
CONFIGURE_FLAGS+= CPPFLAGS="$(ARCH_FLAGS) $(EXTRA_CDEFS)"
CONFIGURE_ACTION+="--without-x"

BUILD_ACTION=$(AUGMENT_LIBRARY_PATH) $(GNUMAKE)
BUILD_DIR=$(CONFIGURE_DIR)

EXTRPATH=LOADER

OUT2LIB+=pango/.libs/libpango-1.0.0.dylib
OUT2LIB+=pango/.libs/libpangocairo-1.0.0.dylib

OUT2BIN+=pango/.libs/pango-querymodules

OUT2INC+=pango/pango-attributes.h
OUT2INC+=pango/pango-enum-types.h
OUT2INC+=pango/pango-glyph.h
OUT2INC+=pango/pango-modules.h
OUT2INC+=pango/pango.h
OUT2INC+=pango/pango-bidi-type.h
OUT2INC+=pango/pango-features.h
OUT2INC+=pango/pango-gravity.h
OUT2INC+=pango/pango-renderer.h
OUT2INC+=pango/pangoatsui.h
OUT2INC+=pango/pango-break.h
OUT2INC+=pango/pango-font.h
OUT2INC+=pango/pango-item.h
OUT2INC+=pango/pango-script.h
OUT2INC+=pango/pangocairo.h
OUT2INC+=pango/pango-context.h
OUT2INC+=pango/pango-fontmap.h
OUT2INC+=pango/pango-language.h
OUT2INC+=pango/pango-tabs.h
OUT2INC+=pango/pango-coverage.h
OUT2INC+=pango/pango-fontset.h
OUT2INC+=pango/pango-layout.h
OUT2INC+=pango/pango-types.h
OUT2INC+=pango/pango-engine.h
OUT2INC+=pango/pango-glyph-item.h
OUT2INC+=pango/pango-matrix.h
OUT2INC+=pango/pango-utils.h

.ELIF "$(OS)"=="WNT"

PATCH_FILES=pango-1.28.3-win32.patch
ADDITIONAL_FILES=config.h module.defs make.msc glib-mkenums.pl msvc_recommended_pragmas.h
CONFIGURE_DIR=
CONFIGURE_ACTION=
BUILD_DIR=./pango
BUILD_ACTION=nmake -f makefile.msc

OUT2LIB+=pango/pango-1.0.lib
OUT2LIB+=pango/pango-1.0s.lib
OUT2LIB+=pango/pangocairo-1.0.lib
OUT2LIB+=pango/pangocairo-1.0s.lib
OUT2LIB+=pango/pangowin32-1.0.lib
OUT2LIB+=pango/pangowin32-1.0s.lib

OUT2BIN+=pango/libpango-1.0-0.dll
OUT2BIN+=pango/libpangocairo-1.0-0.dll
OUT2BIN+=pango/libpangowin32-1.0-0.dll
OUT2BIN+=pango/querymodules.exe

OUT2INC+=pango/pango.h
OUT2INC+=pango/pango-attributes.h
OUT2INC+=pango/pango-bidi-type.h
OUT2INC+=pango/pango-break.h
OUT2INC+=pango/pango-context.h
OUT2INC+=pango/pango-coverage.h
OUT2INC+=pango/pango-engine.h
OUT2INC+=pango/pango-enum-types.h
OUT2INC+=pango/pango-features.h
OUT2INC+=pango/pango-font.h
OUT2INC+=pango/pango-fontmap.h
OUT2INC+=pango/pango-fontset.h
OUT2INC+=pango/pango-glyph.h
OUT2INC+=pango/pango-glyph-item.h
OUT2INC+=pango/pango-gravity.h
OUT2INC+=pango/pango-item.h
OUT2INC+=pango/pango-layout.h
OUT2INC+=pango/pango-matrix.h
OUT2INC+=pango/pango-renderer.h
OUT2INC+=pango/pango-script.h
OUT2INC+=pango/pango-tabs.h
OUT2INC+=pango/pango-types.h
OUT2INC+=pango/pango-utils.h

.ELSE
.ENDIF

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk
