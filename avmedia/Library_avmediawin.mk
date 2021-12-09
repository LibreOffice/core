# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,avmediawin))

$(eval $(call gb_Library_set_componentfile,avmediawin,avmedia/source/win/avmediawin,services))

$(eval $(call gb_Library_use_external,avmediawin,boost_headers))

$(eval $(call gb_Library_set_include,avmediawin,\
	$$(INCLUDE) \
	-I$(SRCDIR)/avmedia/source/inc \
))

$(eval $(call gb_Library_use_sdk_api,avmediawin))

$(eval $(call gb_Library_use_libraries,avmediawin,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	tl \
	utl \
	vcl \
))

$(eval $(call gb_Library_use_system_win32_libs,avmediawin,\
	gdi32 \
	ole32 \
	oleaut32 \
	strmiids \
))

$(eval $(call gb_Library_add_exception_objects,avmediawin,\
	avmedia/source/win/framegrabber \
	avmedia/source/win/manager \
	avmedia/source/win/player \
	avmedia/source/win/window \
))

# vim: set noet sw=4 ts=4:
