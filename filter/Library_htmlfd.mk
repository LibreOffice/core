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

$(eval $(call gb_Library_Library,htmlfd))

$(eval $(call gb_Library_set_componentfile,htmlfd,filter/source/htmlfilterdetect/htmlfd))

$(eval $(call gb_Library_use_external,htmlfd,boost_headers))

$(eval $(call gb_Library_use_sdk_api,htmlfd))

$(eval $(call gb_Library_use_libraries,htmlfd,\
	ucbhelper \
	cppuhelper \
	cppu \
	sal \
	tl \
	utl \
	svt \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,htmlfd,\
	filter/source/htmlfilterdetect/fdcomp \
	filter/source/htmlfilterdetect/filterdetect \
))

# vim: set noet sw=4 ts=4:
