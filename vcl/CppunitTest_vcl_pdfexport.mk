# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,vcl_pdfexport))

$(eval $(call gb_CppunitTest_add_exception_objects,vcl_pdfexport, \
    vcl/qa/cppunit/pdfexport/pdfexport \
))

$(eval $(call gb_CppunitTest_use_sdk_api,vcl_pdfexport))

$(eval $(call gb_CppunitTest_use_libraries,vcl_pdfexport, \
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

$(eval $(call gb_CppunitTest_use_externals,vcl_pdfexport, \
	boost_headers \
	$(if $(filter PDFIUM,$(BUILD_TYPE)),pdfium) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,vcl_pdfexport))

$(eval $(call gb_CppunitTest_use_ure,vcl_pdfexport))
$(eval $(call gb_CppunitTest_use_vcl,vcl_pdfexport))

$(eval $(call gb_CppunitTest_use_rdb,vcl_pdfexport,services))

$(eval $(call gb_CppunitTest_use_custom_headers,vcl_pdfexport,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,vcl_pdfexport))

# assert if font/glyph fallback occurs
$(eval $(call gb_CppunitTest_set_non_application_font_use,vcl_pdfexport,abort))

$(eval $(call gb_CppunitTest_use_more_fonts,vcl_pdfexport))

# vim: set noet sw=4 ts=4:
