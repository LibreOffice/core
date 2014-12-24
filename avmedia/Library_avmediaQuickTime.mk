# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,avmediaQuickTime))

$(eval $(call gb_Library_set_componentfile,avmediaQuickTime,avmedia/source/quicktime/avmediaQuickTime))

$(eval $(call gb_Library_set_include,avmediaQuickTime,\
	$$(INCLUDE) \
	-I$(SRCDIR)/avmedia/source/inc \
))

$(eval $(call gb_Library_use_external,avmediaQuickTime,boost_headers))

$(eval $(call gb_Library_use_sdk_api,avmediaQuickTime));

$(eval $(call gb_Library_use_libraries,avmediaQuickTime,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	tl \
	vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_use_system_darwin_frameworks,avmediaQuickTime,\
	Cocoa \
	QTKit \
))

$(eval $(call gb_Library_add_libs,avmediaQuickTime,\
	-lobjc \
))

$(eval $(call gb_Library_add_objcxxobjects,avmediaQuickTime,\
	avmedia/source/quicktime/framegrabber \
	avmedia/source/quicktime/manager \
	avmedia/source/quicktime/player \
	avmedia/source/quicktime/quicktimeuno \
	avmedia/source/quicktime/window \
))

# vim: set noet sw=4 ts=4:
