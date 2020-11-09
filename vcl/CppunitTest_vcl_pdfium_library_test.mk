# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,vcl_pdfium_library_test))

$(eval $(call gb_CppunitTest_add_exception_objects,vcl_pdfium_library_test, \
    vcl/qa/cppunit/PDFiumLibraryTest \
    vcl/qa/cppunit/PDFDocumentTest \
))

$(eval $(call gb_CppunitTest_use_sdk_api,vcl_pdfium_library_test))

$(eval $(call gb_CppunitTest_use_libraries,vcl_pdfium_library_test, \
	basegfx \
	comphelper \
	cppu \
	cppuhelper \
	sal \
	test \
	unotest \
	utl \
	tl \
	vcl \
))

$(eval $(call gb_CppunitTest_use_externals,vcl_pdfium_library_test, \
	boost_headers \
	$(if $(filter PDFIUM,$(BUILD_TYPE)),pdfium) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,vcl_pdfium_library_test))

$(eval $(call gb_CppunitTest_use_ure,vcl_pdfium_library_test))
$(eval $(call gb_CppunitTest_use_vcl,vcl_pdfium_library_test))

$(eval $(call gb_CppunitTest_use_rdb,vcl_pdfium_library_test,services))

$(eval $(call gb_CppunitTest_use_configuration,vcl_pdfium_library_test))

# vim: set noet sw=4 ts=4:
