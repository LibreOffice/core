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

PRJNAME=gdk-pixbuf
TARGET=so_gdk-pixbuf

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(SYSTEM_GDKPIXBUF)" == "YES"
all:
	@echo "An already available installation of gdk-pixbuf should exist on your system."
	@echo "Therefore the version provided here does not need to be built in addition."
.ENDIF

# --- Files --------------------------------------------------------

GDKPIXBUFVERSION=2.23.0

TARFILE_NAME=$(PRJNAME)-$(GDKPIXBUFVERSION)
TARFILE_MD5=a7d6c5f2fe2d481149ed3ba807b5c043
               
.IF "$(OS)"=="MACOSX"

PATCH_FILES=gdk-pixbuf-2.23.0.patch

LOADER_PATTERN:=-Wl,-dylib_file,@loader_path/REPLACEME:$(SOLARLIBDIR)/REPLACEME
LOADER_LIBS:=glib-2.0.0 gmodule-2.0.0 gobject-2.0.0 gthread-2.0.0
EXTRA_LINKFLAGS+=$(foreach,lib,$(LOADER_LIBS) $(subst,REPLACEME,lib$(lib).dylib $(LOADER_PATTERN)))

CONFIGURE_DIR=
CONFIGURE_ACTION=$(AUGMENT_LIBRARY_PATH) \
                BASE_DEPENDENCIES_CFLAGS="-I$(SOLARINCDIR)/external -I$(SOLARINCDIR)/external/glib-2.0" \
                BASE_DEPENDENCIES_LIBS=" " \
		GDK_PIXBUF_XLIB_DEP_CFLAGS="-I$(SOLARINCDIR)$/external -I$(SOLARINCDIR)$/external/glib-2.0" \
		GDK_PIXBUF_XLIB_DEP_LIBS=" " \
                 ./configure --disable-dependency-tracking $(eq,$(VERBOSE),$(NULL) --enable-silent-rules --disable-silent-rules) \
                 CPPFLAGS="$(EXTRA_CDEFS) -I$(SOLARINCDIR)/external -I$(SOLARINCDIR)/external/glib-2.0 -I$(SOLARINCDIR)/external/libpng -I$(SOLARINCDIR)/external/jpeg" \
                 CFLAGS="$(ARCH_FLAGS) $(EXTRA_CFLAGS)" \
                 LDFLAGS="$(EXTRA_LINKFLAGS) -L$(SOLARLIBDIR) -lgobject-2.0 -lgio-2.0 -lgthread-2.0 -lgmodule-2.0 -lglib-2.0" \
                 --prefix=/@.__________________________________________________$(EXTRPATH) \
                 --disable-nls \
                 --disable-modules \
                 --with-included-loaders=ani,icns,pcx,ras,tga,png,pnm,wbmp,xbm,xpm,qtif,bmp,gif,ico,jpeg \
                 --disable-glibtest --without-libtiff --without-libjasper

.IF "$(CROSS_COMPILING)"=="YES"
CONFIGURE_FLAGS+=--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM) gio_can_sniff=no
.ENDIF

BUILD_ACTION=$(AUGMENT_LIBRARY_PATH) $(GNUMAKE)
BUILD_DIR=$(CONFIGURE_DIR)

OUT2LIB+=gdk-pixbuf/.libs/libgdk_pixbuf-2.0.0.dylib

OUT2INC+=gdk-pixbuf/gdk-pixbuf-animation.h
OUT2INC+=gdk-pixbuf/gdk-pixbuf-features.h
OUT2INC+=gdk-pixbuf/gdk-pixbuf-marshal.h
OUT2INC+=gdk-pixbuf/gdk-pixbuf.h
OUT2INC+=gdk-pixbuf/gdk-pixbuf-core.h
OUT2INC+=gdk-pixbuf/gdk-pixbuf-io.h
OUT2INC+=gdk-pixbuf/gdk-pixbuf-simple-anim.h
OUT2INC+=gdk-pixbuf/gdk-pixdata.h
OUT2INC+=gdk-pixbuf/gdk-pixbuf-enum-types.h
OUT2INC+=gdk-pixbuf/gdk-pixbuf-loader.h
OUT2INC+=gdk-pixbuf/gdk-pixbuf-transform.h

.ELIF "$(OS)"=="WNT"

PATCH_FILES=gdk-pixbuf-2.23.0-win32.patch
ADDITIONAL_FILES=config.h.win32 glib-mkenums.pl msvc_recommended_pragmas.h 
CONFIGURE_DIR=
CONFIGURE_ACTION=
BUILD_DIR=./gdk-pixbuf
BUILD_ACTION=unset debug; nmake -f makefile.msc

OUT2LIB+=gdk-pixbuf/gdk_pixbuf-2.0.lib

OUT2BIN+=gdk-pixbuf/libgdk_pixbuf-2.0-0.dll

OUT2INC+=gdk-pixbuf/gdk-pixbuf-animation.h
OUT2INC+=gdk-pixbuf/gdk-pixbuf-features.h
OUT2INC+=gdk-pixbuf/gdk-pixbuf-marshal.h
OUT2INC+=gdk-pixbuf/gdk-pixbuf.h
OUT2INC+=gdk-pixbuf/gdk-pixbuf-core.h
OUT2INC+=gdk-pixbuf/gdk-pixbuf-io.h
OUT2INC+=gdk-pixbuf/gdk-pixbuf-simple-anim.h
OUT2INC+=gdk-pixbuf/gdk-pixdata.h
OUT2INC+=gdk-pixbuf/gdk-pixbuf-enum-types.h
OUT2INC+=gdk-pixbuf/gdk-pixbuf-loader.h
OUT2INC+=gdk-pixbuf/gdk-pixbuf-transform.h

.ELSE

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk
