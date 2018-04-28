# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,vcl_graphic_test))

$(eval $(call gb_CppunitTest_add_exception_objects,vcl_graphic_test, \
    vcl/qa/cppunit/GraphicTest \
    vcl/qa/cppunit/GraphicDescriptorTest \
))

$(eval $(call gb_CppunitTest_use_externals,vcl_graphic_test,\
	boost_headers \
	glm_headers \
))

$(eval $(call gb_CppunitTest_set_include,vcl_graphic_test,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_CppunitTest_use_libraries,vcl_graphic_test, \
	comphelper \
	cppu \
	cppuhelper \
	sal \
    svt \
	test \
	tl \
	unotest \
	vcl \
	utl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,vcl_graphic_test))

$(eval $(call gb_CppunitTest_use_ure,vcl_graphic_test))
$(eval $(call gb_CppunitTest_use_vcl,vcl_graphic_test))

$(eval $(call gb_CppunitTest_use_components,vcl_graphic_test,\
    configmgr/source/configmgr \
    i18npool/util/i18npool \
    ucb/source/core/ucb1 \
    unotools/util/utl \
))

$(eval $(call gb_CppunitTest_use_configuration,vcl_graphic_test))

# vim: set noet sw=4 ts=4:
