# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,avmediagst_0_10))

$(eval $(call gb_Library_set_componentfile,avmediagst_0_10,avmedia/source/gstreamer/avmediagstreamer_0_10))

$(eval $(call gb_Library_set_include,avmediagst_0_10,\
	$$(INCLUDE) \
	-I$(SRCDIR)/avmedia/source/inc \
	$(GSTREAMER_0_10_CFLAGS) \
))

$(eval $(call gb_Library_use_externals,avmediagst_0_10,\
	boost_headers \
	libxml2 \
))

$(eval $(call gb_Library_use_sdk_api,avmediagst_0_10))

$(eval $(call gb_Library_use_libraries,avmediagst_0_10,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	tl \
	vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_libs,avmediagst_0_10,\
	$(GSTREAMER_0_10_LIBS) \
	-lgstinterfaces-0.10 \
))

$(eval $(call gb_Library_add_exception_objects,avmediagst_0_10,\
	avmedia/source/gstreamer/gst_0_10 \
))

# vim: set noet sw=4 ts=4:
