# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,cached1))

$(eval $(call gb_Library_set_componentfile,cached1,ucb/source/cacher/cached1,services))

$(eval $(call gb_Library_use_external,cached1,boost_headers))

$(eval $(call gb_Library_use_sdk_api,cached1))

$(eval $(call gb_Library_use_libraries,cached1,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	ucbhelper \
))

$(eval $(call gb_Library_add_exception_objects,cached1,\
	ucb/source/cacher/cachedcontentresultset \
	ucb/source/cacher/cachedcontentresultsetstub \
	ucb/source/cacher/cacheddynamicresultset \
	ucb/source/cacher/cacheddynamicresultsetstub \
	ucb/source/cacher/contentresultsetwrapper \
	ucb/source/cacher/dynamicresultsetwrapper \
))

# vim: set noet sw=4 ts=4:
