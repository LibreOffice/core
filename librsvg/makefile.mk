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

PRJNAME=librsvg
TARGET=so_librsvg

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(SYSTEM_LIBRSVG)" == "YES"
all:
	@echo "An already available installation of librsvg should exist on your system."
	@echo "Therefore the version provided here does not need to be built in addition."
.ENDIF

# --- Files --------------------------------------------------------

LIBRSVGVERSION=2.32.1

TARFILE_NAME=$(PRJNAME)-$(LIBRSVGVERSION)
TARFILE_MD5=d7a242ca43e33e1b63d3073f9d46a6a8

.IF "$(OS)" == "MACOSX" || "$(OS)" == "WNT"

.IF "$(OS)" == "MACOSX"

PATCH_FILES=librsvg-2.32.1.patch

LOADER_PATTERN:=-Wl,-dylib_file,@loader_path/REPLACEME:$(SOLARLIBDIR)/REPLACEME
LOADER_LIBS:=cairo.2 gio-2.0.0 glib-2.0.0 gmodule-2.0.0 gobject-2.0.0 gthread-2.0.0 intl.8 pango-1.0.0
EXTRA_LINKFLAGS+=$(foreach,lib,$(LOADER_LIBS) $(subst,REPLACEME,lib$(lib).dylib $(LOADER_PATTERN)))

.IF "$(LIBXML_LIBS)" == ""
LIBXML_LIBS=-lxml2
.ENDIF

.IF "$(SYSTEM_LIBXML)" == "YES"
my_libxml2_cflags=$(LIBXML_CFLAGS)
my_libxml2_libs=$(LIBXML_LIBS)
my_dylib_file=
.ELSE
my_libxml2_cflags=-I$(SOLARINCDIR)/external/libxml
my_libxml2_libs=-L$(SOLARLIBDIR) -lxml2
my_dylib_file="-Wl,-dylib_file,@loader_path/../ure-link/lib/libxml2.2.dylib:$(SOLARLIBDIR)/libxml2.2.dylib"
.ENDIF

CONFIGURE_LDFLAGS=-L$(SOLARLIBDIR) $(eq,$(OS),MACOSX $(EXTRA_LINKFLAGS) $(NULL))
CONFIGURE_DIR=
CONFIGURE_ACTION=$(AUGMENT_LIBRARY_PATH) ./configure \
                 --prefix=/@.__________________________________________________$(EXTRPATH)
CONFIGURE_FLAGS=--disable-gtk-theme --disable-tools --with-croco --with-svgz \
                 --disable-pixbuf-loader --disable-dependency-tracking $(eq,$(VERBOSE),$(NULL) --enable-silent-rules --disable-silent-rules) \
                 LIBRSVG_CFLAGS="-I$(SOLARINCDIR)/external/glib-2.0 -I$(SOLARINCDIR)/external/gdk-pixbuf-2.0 -I$(SOLARINCDIR)/external/pango-1.0 -I$(SOLARINCDIR)/cairo $(my_libxml2_cflags)" \
                 LIBRSVG_LIBS="-L$(SOLARLIBDIR) -lgdk_pixbuf-2.0 -lpango-1.0 -lpangocairo-1.0 -lgthread-2.0 -lgio-2.0 -lgmodule-2.0 -lgobject-2.0 -lglib-2.0 $(my_libxml2_libs) -lcairo -lintl" \
                 GDK_PIXBUF_CFLAGS="-I$(SOLARINCDIR)/external/gdk-pixbuf-2.0" \
                 GDK_PIXBUF_LIBS=-lgdk_pixbuf-2.0 \
                 GTHREAD_CFLAGS=-I$(SOLARINCDIR)/external/glib-2.0 \
                 GTHREAD_LIBS=-lgthread-2.0 \
                 LIBCROCO_CFLAGS="-I$(SOLARINCDIR)/external/libcroco-0.6" \
                 LIBCROCO_LIBS=-lcroco-0.6 \
                 CFLAGS="$(ARCH_FLAGS) $(EXTRA_CFLAGS) -I$(SOLARINCDIR)/external -I$(SOLARINCDIR)/external/glib-2.0 -I$(SOLARINCDIR)/external/gdk-pixbuf-2.0 -I$(SOLARINCDIR)/external/pango-1.0 -I$(SOLARINCDIR)/cairo" \
                 LDFLAGS="$(CONFIGURE_LDFLAGS) $(my_dylib_file)"

CONFIGURE_FLAGS+= CPPFLAGS="$(ARCH_FLAGS) $(EXTRA_CDEFS)"

.IF "$(CROSS_COMPILING)"=="YES"
CONFIGURE_FLAGS+=--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)
.ENDIF

BUILD_ACTION=$(AUGMENT_LIBRARY_PATH) \
             $(GNUMAKE) $(!eq,$(VERBOSE),$(NULL) V=1) -j$(MAXPROCESS)
BUILD_DIR=$(CONFIGURE_DIR)

OUT2LIB+=.libs/librsvg-2.2.dylib

OUT2INC+=librsvg-enum-types.h
OUT2INC+=librsvg-features.h
OUT2INC+=rsvg-cairo.h
OUT2INC+=rsvg.h

.ELIF "$(OS)"=="WNT"

PATCH_FILES=librsvg-2.32.1-win32.patch
ADDITIONAL_FILES=config.h makefile.mk

BUILD_DIR=.
BUILD_ACTION=dmake

OUT2INC+=librsvg-enum-types.h
OUT2INC+=librsvg-features.h
OUT2INC+=rsvg-cairo.h
OUT2INC+=rsvg.h

.ENDIF

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk
