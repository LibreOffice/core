# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Generate gobject-introspection files for the kitgtk library.
# These are not packaged because there is no good place to put them
# where the system will actually find them and where it won't conflict with a
# distro packaged office suite; on Fedora 30 at least there's no /opt path in
# $XDG_DATA_DIRS

introspection_WORKDIR := $(gb_CustomTarget_workdir)/kit/introspection

$(eval $(call gb_CustomTarget_CustomTarget,kit/introspection))

$(eval $(call gb_CustomTarget_register_targets,kit/introspection,\
	KitDocumentView-0.1.gir \
	KitDocumentView-0.1.typelib \
))

$(introspection_WORKDIR)/KitDocumentView-0.1.gir: \
		$(call gb_Library_get_target,kitgtk)
	mkdir -p $(dir $@)
	PYTHONWARNINGS=default g-ir-scanner "${SRCDIR}/include/COKit/COKitGtk.h" \
				 "${SRCDIR}/kit/source/gtk/kitdocview.cxx" \
                 `${PKG_CONFIG} --cflags-only-I gobject-introspection-1.0 gtk+-3.0` \
				 -I"${SRCDIR}/include/" \
                 --include=GLib-2.0 --include=GObject-2.0 --include=Gio-2.0 \
                 --library=kitgtk --library-path="${INSTDIR}/program" \
                 --include=Gdk-3.0 --include=GdkPixbuf-2.0 --include=Gtk-3.0 \
                 --namespace=KitDocumentView --nsversion=0.1 --identifier-prefix=KitDoc --symbol-prefix=kit_doc \
                 --c-include="COKit/COKitGtk.h" \
				 --output="$@" --warn-all --no-libtool

$(introspection_WORKDIR)/KitDocumentView-0.1.typelib: $(introspection_WORKDIR)/KitDocumentView-0.1.gir
	g-ir-compiler "$<" --output="$@"

# vim: set noet sw=4 ts=4:
