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

$(eval $(call gb_Library_Library,vcleventfilter))

$(eval $(call gb_Library_set_componentfile,vcleventfilter,filter/source/vcleventfilter/vcleventfilter))

$(eval $(call gb_Library_use_external,vcleventfilter,boost_headers))

$(eval $(call gb_Library_use_sdk_api,vcleventfilter))

$(eval $(call gb_Library_use_libraries,vcleventfilter,\
	comphelper \
	ucbhelper \
	cppuhelper \
	cppu \
	sal \
	tl \
	utl \
	svt \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,vcleventfilter,\
	filter/source/vcleventfilter/vcleventfilter \
))

# vim: set noet sw=4 ts=4:
