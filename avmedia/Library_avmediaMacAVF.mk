# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,avmediaMacAVF))

$(eval $(call gb_Library_set_componentfile,avmediaMacAVF,avmedia/source/macavf/avmediaMacAVF,services))

$(eval $(call gb_Library_set_include,avmediaMacAVF,\
	$$(INCLUDE) \
	-I$(SRCDIR)/avmedia/source/inc \
))

$(eval $(call gb_Library_use_external,avmediaMacAVF,boost_headers))

$(eval $(call gb_Library_use_sdk_api,avmediaMacAVF));

$(eval $(call gb_Library_use_libraries,avmediaMacAVF,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	tl \
	vcl \
))

$(eval $(call gb_Library_use_system_darwin_frameworks,avmediaMacAVF,\
	AVFoundation \
	Cocoa \
	CoreMedia \
))

$(eval $(call gb_Library_add_objcxxobjects,avmediaMacAVF,\
	avmedia/source/macavf/framegrabber \
	avmedia/source/macavf/manager \
	avmedia/source/macavf/player \
	avmedia/source/macavf/window \
))

# vim: set noet sw=4 ts=4:
