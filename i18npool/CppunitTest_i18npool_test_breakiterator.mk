# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,i18npool_test_breakiterator))

$(eval $(call gb_CppunitTest_use_external,i18npool_test_breakiterator,icu_headers))

$(eval $(call gb_CppunitTest_use_api,i18npool_test_breakiterator,\
	udkapi \
	offapi \
))

$(eval $(call gb_CppunitTest_use_libraries,i18npool_test_breakiterator,\
	cppu \
	cppuhelper \
	sal \
	unotest \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_add_exception_objects,i18npool_test_breakiterator,\
    i18npool/qa/cppunit/test_breakiterator \
))

$(eval $(call gb_CppunitTest_use_ure,i18npool_test_breakiterator))

$(eval $(call gb_CppunitTest_use_components,i18npool_test_breakiterator,\
	i18npool/util/i18npool \
))

$(call gb_CppunitTest_get_target,i18npool_test_breakiterator) : \
    $(call gb_Library_get_target,dict_ja) \
    $(call gb_Library_get_target,dict_zh)

# vim: set noet sw=4 ts=4:
