# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#*************************************************************************

$(eval $(call gb_Library_Library,textfd))

$(eval $(call gb_Library_set_componentfile,textfd,filter/source/textfilterdetect/textfd,services))

$(eval $(call gb_Library_use_external,textfd,boost_headers))

$(eval $(call gb_Library_use_sdk_api,textfd))

$(eval $(call gb_Library_use_libraries,textfd,\
	comphelper \
	ucbhelper \
	cppuhelper \
	cppu \
	sal \
	sfx \
	tl \
	utl \
	svt \
))

$(eval $(call gb_Library_add_exception_objects,textfd,\
	filter/source/textfilterdetect/filterdetect \
))

# vim: set noet sw=4 ts=4:
