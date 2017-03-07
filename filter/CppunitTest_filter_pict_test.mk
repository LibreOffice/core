# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
$(eval $(call gb_CppunitTest_CppunitTest,filter_pict_test))

$(eval $(call gb_CppunitTest_use_externals,filter_pict_test,\
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_add_exception_objects,filter_pict_test, \
    filter/qa/cppunit/filters-pict-test \
))

$(eval $(call gb_CppunitTest_use_libraries,filter_pict_test, \
	cppu \
	comphelper \
	basegfx \
	sal \
	test \
	tk \
	tl \
	utl \
	unotest \
	vcl \
))

$(eval $(call gb_CppunitTest_use_library_objects,filter_pict_test, \
    gie \
))

$(eval $(call gb_CppunitTest_use_sdk_api,filter_pict_test))

$(eval $(call gb_CppunitTest_use_ure,filter_pict_test))
$(eval $(call gb_CppunitTest_use_vcl,filter_pict_test))

$(eval $(call gb_CppunitTest_use_components,filter_pict_test,\
    configmgr/source/configmgr \
    i18npool/util/i18npool \
))

$(eval $(call gb_CppunitTest_use_configuration,filter_pict_test))

# vim: set noet sw=4 ts=4:
