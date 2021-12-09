# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,srtrs1))

$(eval $(call gb_Library_set_componentfile,srtrs1,ucb/source/sorter/srtrs1,services))

$(eval $(call gb_Library_use_external,srtrs1,boost_headers))

$(eval $(call gb_Library_use_sdk_api,srtrs1))

$(eval $(call gb_Library_use_libraries,srtrs1,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	tl \
	ucbhelper \
))

$(eval $(call gb_Library_add_exception_objects,srtrs1,\
	ucb/source/sorter/sortdynres \
	ucb/source/sorter/sortresult \
))

# vim: set noet sw=4 ts=4:
