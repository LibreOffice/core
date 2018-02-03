# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,vcl_color))

$(eval $(call gb_CppunitTest_set_include,vcl_color,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_CppunitTest_add_exception_objects,vcl_color, \
	vcl/qa/cppunit/test_color \
))

$(eval $(call gb_CppunitTest_use_externals,vcl_color,boost_headers))

$(eval $(call gb_CppunitTest_use_libraries,vcl_color, \
	comphelper \
	cppu \
	cppuhelper \
	sal \
	svt \
	test \
	tl \
	unotest \
	vcl \
    sal \
))

$(eval $(call gb_CppunitTest_use_sdk_api,vcl_color))

$(eval $(call gb_CppunitTest_use_ure,vcl_color))
$(eval $(call gb_CppunitTest_use_vcl,vcl_color))

$(eval $(call gb_CppunitTest_use_components,vcl_color,\
	configmgr/source/configmgr \
	i18npool/util/i18npool \
	ucb/source/core/ucb1 \
))

$(eval $(call gb_CppunitTest_use_configuration,vcl_color))

# vim: set noet sw=4 ts=4:
