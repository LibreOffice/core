# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,i18nsearch))

$(eval $(call gb_Library_set_componentfile,i18nsearch,i18npool/source/search/i18nsearch,services))

$(eval $(call gb_Library_use_sdk_api,i18nsearch))

$(eval $(call gb_Library_use_externals,i18nsearch,\
	icui18n \
	icuuc \
	icu_headers \
))

$(eval $(call gb_Library_use_libraries,i18nsearch,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
))

$(eval $(call gb_Library_add_exception_objects,i18nsearch,\
	i18npool/source/search/levdis \
	i18npool/source/search/textsearch \
))

# vim: set noet sw=4 ts=4:
