# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,i18npool_test_textsearch))

$(eval $(call gb_CppunitTest_use_api,i18npool_test_textsearch,\
	udkapi \
	offapi \
))

$(eval $(call gb_CppunitTest_use_ure,i18npool_test_textsearch))

$(eval $(call gb_CppunitTest_add_exception_objects,i18npool_test_textsearch,\
	i18npool/qa/cppunit/test_textsearch \
))

$(eval $(call gb_CppunitTest_use_components,i18npool_test_textsearch,\
	i18npool/source/search/i18nsearch \
	i18npool/util/i18npool \
))

$(eval $(call gb_CppunitTest_use_externals,i18npool_test_textsearch,\
	icui18n \
	icuuc \
	icu_headers \
))

$(eval $(call gb_CppunitTest_use_libraries,i18npool_test_textsearch,\
	cppu \
	cppuhelper \
	sal \
	unotest \
	$(gb_UWINAPI) \
))

# vim: set noet sw=4 ts=4:
