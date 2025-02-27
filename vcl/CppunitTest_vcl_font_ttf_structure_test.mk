# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,vcl_font_ttf_structure_test))

$(eval $(call gb_CppunitTest_set_include,vcl_font_ttf_structure_test,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_CppunitTest_add_exception_objects,vcl_font_ttf_structure_test, \
	vcl/qa/cppunit/font/TTFStructureTest \
))

$(eval $(call gb_CppunitTest_use_externals,vcl_font_ttf_structure_test,boost_headers))

$(eval $(call gb_CppunitTest_use_libraries,vcl_font_ttf_structure_test, \
	basegfx \
	comphelper \
	cppu \
	cppuhelper \
	sal \
	subsequenttest \
	test \
	unotest \
	utl \
	tl \
	vcl \
	xmlsecurity \
))

$(eval $(call gb_CppunitTest_use_sdk_api,vcl_font_ttf_structure_test))
$(eval $(call gb_CppunitTest_use_ure,vcl_font_ttf_structure_test))
$(eval $(call gb_CppunitTest_use_vcl,vcl_font_ttf_structure_test))
$(eval $(call gb_CppunitTest_use_rdb,vcl_font_ttf_structure_test,services))
$(eval $(call gb_CppunitTest_use_configuration,vcl_font_ttf_structure_test))

# vim: set noet sw=4 ts=4:
