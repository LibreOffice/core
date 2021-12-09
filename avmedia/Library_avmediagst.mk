# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,avmediagst))

$(eval $(call gb_Library_set_componentfile,avmediagst,avmedia/source/gstreamer/avmediagstreamer,services))

$(eval $(call gb_Library_set_include,avmediagst,\
	$$(INCLUDE) \
	-I$(SRCDIR)/avmedia/source/inc \
	$(GSTREAMER_1_0_CFLAGS) \
))

$(eval $(call gb_Library_add_libs,avmediagst,\
    $(GSTREAMER_1_0_LIBS) \
))

$(eval $(call gb_Library_use_external,avmediagst,boost_headers))

$(eval $(call gb_Library_use_sdk_api,avmediagst))

$(eval $(call gb_Library_use_libraries,avmediagst,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	salhelper \
	tl \
	vcl \
))

$(eval $(call gb_Library_add_exception_objects,avmediagst,\
	avmedia/source/gstreamer/gstmanager \
	avmedia/source/gstreamer/gstplayer \
	avmedia/source/gstreamer/gstwindow \
	avmedia/source/gstreamer/gstframegrabber \
))

# vim: set noet sw=4 ts=4:
