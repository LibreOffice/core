# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,librsvg))

$(eval $(call gb_ExternalProject_use_unpacked,librsvg,rsvg))

$(eval $(call gb_ExternalProject_register_targets,librsvg,\
	build \
))

$(call gb_ExternalProject_get_state_target,librsvg,build) :
	cd $(EXTERNAL_WORKDIR) \
	&& ./configure --disable-gtk-theme --disable-tools --with-croco --with-svgz \
    --disable-pixbuf-loader --disable-dependency-tracking \
	$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
	LIBCROCO_CFLAGS="-I$(OUTDIR)/inc/external/libcroco-0.6" \
    LIBCROCO_LIBS="-lcroco-0.6" \
	GTHREAD_CFLAGS="-I$(OUTDIR)/inc/external/glib-2.0" \
    GTHREAD_LIBS="-lgthread-2.0" \
	GDK_PIXBUF_CFLAGS="-I$(OUTDIR)/inc/external/gdk-pixbuf-2.0" \
    GDK_PIXBUF_LIBS="-lgdk_pixbuf-2.0" \
	LIBRSVG_CFLAGS="-I$(OUTDIR)/inc/external/glib-2.0 -I$(OUTDIR)/inc/external/gdk-pixbuf-2.0 -I$(OUTDIR)/inc/external/pango-1.0 -I$(OUTDIR)/inc/cairo $(if $(filter YES,$(SYSTEM_LIBXML)),$(LIBXML_CFLAGS),-I$(OUTDIR)/inc/externel/libxml)" \
    LIBRSVG_LIBS="-L$(OUTDIR)/lib -lgdk_pixbuf-2.0 -lpango-1.0 -lpangocairo-1.0 -lgthread-2.0 -lgio-2.0 -lgmodule-2.0 -lgobject-2.0 -lglib-2.0 $(if $(filter YES,$(SYSTEM_LIBXML)),$(LIBXML_LIBS),-lxml2) -lcairo -lintl" \
	CFLAGS="-I$(OUTDIR)/inc/external -I$(OUTDIR)/inc/external/glib-2.0 -I$(OUTDIR)/inc/external/gdk-pixbuf-2.0 -I$(OUTDIR)/inc/external/pango-1.0 -I$(OUTDIR)/inc/cairo" \
	LDFLAGS="$(foreach lib,cairo.2 gio-2.0.0 glib-2.0.0 gmodule-2.0.0 gobject-2.0.0 gthread-2.0.0 intl.8 pango-1.0.0,-Wl,-dylib_file,@loader_path/lib$(lib).dylib:$(OUTDIR)/lib/lib$(lib).dylib) $(if $(filter YES,$(SYSTEM_LIBXML)),,-Wl,-dylib_file,@loader_path/../ure-link/lib/libxml2.2.dylib:$(OUTDIR)/lib/libxml2.2.dylib)" \
	&& PATH=$(OUTDIR_FOR_BUILD)/bin:$$PATH $(MAKE) \
	&& touch $@
# vim: set noet sw=4 ts=4:
