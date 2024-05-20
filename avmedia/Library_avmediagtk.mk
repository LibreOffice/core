# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,avmediagtk))

$(eval $(call gb_Library_set_componentfile,avmediagtk,avmedia/source/gtk/avmediagtk,services))

$(eval $(call gb_Library_set_include,avmediagtk,\
	$$(INCLUDE) \
	$$(GTK4_CFLAGS) \
	-I$(SRCDIR)/avmedia/source/inc \
	-I$(SRCDIR)/avmedia/source/gstreamer \
))

$(eval $(call gb_Library_use_external,avmediagtk,boost_headers))

$(eval $(call gb_Library_use_sdk_api,avmediagtk))

$(eval $(call gb_Library_use_libraries,avmediagtk,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	salhelper \
	tl \
	vcl \
))

$(eval $(call gb_Library_add_libs,avmediagtk,\
    $(GTK4_LIBS) \
))

$(eval $(call gb_Library_add_exception_objects,avmediagtk,\
	avmedia/source/gtk/gstwindow \
	avmedia/source/gtk/gtkmanager \
	avmedia/source/gtk/gtkplayer \
))

# vim: set noet sw=4 ts=4:
