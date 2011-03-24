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

.IF "$(OS)"=="MACOSX" || "$(OS)"=="WNT"

PRJ=.

PRJNAME=libgsf
TARGET=so_libgsf

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(SYSTEM_LIBGSF)" == "YES"
all:
    @echo "An already available installation of gdk-pixbuf should exist on your system."
    @echo "Therefore the version provided here does not need to be built in addition."
.ENDIF

# --- Files --------------------------------------------------------

LIBGSFVERSION=1.14.19

TARFILE_NAME=$(PRJNAME)-$(LIBGSFVERSION)
TARFILE_MD5=3a84ac2da37cae5bf7ce616228c6fbde

PATCH_FILES=libgsf-1.14.19.patch

CONFIGURE_DIR=
CONFIGURE_ACTION=$(AUGMENT_LIBRARY_PATH) \
                 .$/configure \
                 --prefix=$(SRC_ROOT)$/$(PRJNAME)$/$(MISC) \
                 CFLAGS="$(ARCH_FLAGS) $(EXTRA_CFLAGS) -I$(SOLARINCDIR)$/external -I$(SOLARINCDIR)$/external$/glib-2.0" \
                 LDFLAGS="-L$(SOLARLIBDIR)" \
                 --without-python \
                 --without-bonobo \
                 --with-bz2 \
                 --with-gio \
                 --with-gdk_pixbuf \
                 --without-gnome-vfs \
                 --disable-nls \
                 LIBGSF_CFLAGS="-I$(SOLARINCDIR)$/external$/glib-2.0" \
                 LIBGSF_LIBS="-lxml2 -lgio-2.0 -lgobject-2.0 -lgmodule-2.0 -lgthread-2.0 -lglib-2.0 -lintl" \
                 LIBGSF_GIO_CFLAGS="-I$(SOLARINCDIR)$/external$/glib-2.0" \
                 LIBGSF_GIO_LIBS="-lgio-2.0 -lgmodule-2.0 -lgthread-2.0 -lglib-2.0 -lintl" \
                 GDK_PIXBUF_CFLAGS="-I$(SOLARINCDIR)$/external$/gdk-pixbuf-2.0" \
                 GDK_PIXBUF_LIBS="-lgdk_pixbuf-2.0"
                 
                 
CONFIGURE_FLAGS=$(eq,$(OS),MACOSX CPPFLAGS="$(EXTRA_CDEFS)" $(NULL))
                
BUILD_ACTION=$(AUGMENT_LIBRARY_PATH) \
             $(GNUMAKE)
BUILD_DIR=$(CONFIGURE_DIR)

.IF "$(OS)"=="MACOSX"
EXTRPATH=LOADER
OUT2LIB+=gsf$/.libs$/libgsf-1.114.dylib

OUT2INC+=gsf$/gsf-blob.h
OUT2INC+=gsf$/gsf-input-impl.h
OUT2INC+=gsf$/gsf-outfile.h
OUT2INC+=gsf$/gsf-clip-data.h
OUT2INC+=gsf$/gsf-input-iochannel.h
OUT2INC+=gsf$/gsf-output-bzip.h
OUT2INC+=gsf$/gsf-doc-meta-data.h
OUT2INC+=gsf$/gsf-input-memory.h
OUT2INC+=gsf$/gsf-output-csv.h
OUT2INC+=gsf$/gsf-docprop-vector.h
OUT2INC+=gsf$/gsf-input-proxy.h
OUT2INC+=gsf$/gsf-output-gio.h
OUT2INC+=gsf$/gsf-impl-utils.h
OUT2INC+=gsf$/gsf-input-stdio.h
OUT2INC+=gsf$/gsf-output-gzip.h
OUT2INC+=gsf$/gsf-infile-impl.h
OUT2INC+=gsf$/gsf-input-textline.h
OUT2INC+=gsf$/gsf-output-iconv.h
OUT2INC+=gsf$/gsf-infile-msole.h
OUT2INC+=gsf$/gsf-input.h
OUT2INC+=gsf$/gsf-output-impl.h
OUT2INC+=gsf$/gsf-infile-msvba.h
OUT2INC+=gsf$/gsf-libxml.h
OUT2INC+=gsf$/gsf-output-iochannel.h
OUT2INC+=gsf$/gsf-infile-stdio.h
OUT2INC+=gsf$/gsf-meta-names.h
OUT2INC+=gsf$/gsf-output-memory.h
OUT2INC+=gsf$/gsf-infile-tar.h
OUT2INC+=gsf$/gsf-msole-utils.h
OUT2INC+=gsf$/gsf-output-stdio.h
OUT2INC+=gsf$/gsf-infile-zip.h
OUT2INC+=gsf$/gsf-open-pkg-utils.h
OUT2INC+=gsf$/gsf-output.h
OUT2INC+=gsf$/gsf-infile.h
OUT2INC+=gsf$/gsf-opendoc-utils.h
OUT2INC+=gsf$/gsf-shared-memory.h
OUT2INC+=gsf$/gsf-input-bzip.h
OUT2INC+=gsf$/gsf-outfile-impl.h
OUT2INC+=gsf$/gsf-structured-blob.h
OUT2INC+=gsf$/gsf-input-gio.h
OUT2INC+=gsf$/gsf-outfile-msole.h
OUT2INC+=gsf$/gsf-timestamp.h
OUT2INC+=gsf$/gsf-input-gzip.h
OUT2INC+=gsf$/gsf-outfile-stdio.h
OUT2INC+=gsf$/gsf-utils.h
OUT2INC+=gsf$/gsf-input-http.h
OUT2INC+=gsf$/gsf-outfile-zip.h
OUT2INC+=gsf$/gsf.h
.ELIF "$(OS)"=="WNT"
.ELSE
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

.ENDIF

