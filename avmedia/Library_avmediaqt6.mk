# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,avmediaqt6))

$(eval $(call gb_Library_set_componentfile,avmediaqt6,avmedia/source/qt6/avmediaqt,services))

$(eval $(call gb_Library_use_custom_headers,avmediaqt6,avmedia/source/qt6))

$(eval $(call gb_Library_set_include,avmediaqt6,\
	$$(INCLUDE) \
	-I$(SRCDIR)/avmedia/source/inc \
	-I$(SRCDIR)/avmedia/source/gstreamer \
))

$(eval $(call gb_Library_use_externals,avmediaqt6,\
	qt6 \
))

$(eval $(call gb_Library_use_sdk_api,avmediaqt6))

$(eval $(call gb_Library_use_libraries,avmediaqt6,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	salhelper \
	tl \
	vcl \
))

$(eval $(call gb_Library_add_exception_objects,avmediaqt6,\
	avmedia/source/qt6/gstwindow \
	avmedia/source/qt6/QtFrameGrabber \
	avmedia/source/qt6/QtManager \
	avmedia/source/qt6/QtPlayer \
))

# vim: set noet sw=4 ts=4:
