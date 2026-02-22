# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,vcl_pdf_encryption))

$(eval $(call gb_CppunitTest_add_exception_objects,vcl_pdf_encryption, \
    vcl/qa/cppunit/pdfexport/PDFEncryptionTest \
))

$(eval $(call gb_CppunitTest_set_include,vcl_pdf_encryption,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_CppunitTest_use_libraries,vcl_pdf_encryption, \
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

$(eval $(call gb_CppunitTest_use_externals,vcl_pdf_encryption, \
	boost_headers \
	libxml2 \
	$(if $(filter PDFIUM,$(BUILD_TYPE)),pdfium) \
))

ifeq ($(TLS),NSS)
$(eval $(call gb_CppunitTest_use_externals,vcl_pdf_encryption,\
       plc4 \
       nss3 \
))
endif

$(eval $(call gb_CppunitTest_use_sdk_api,vcl_pdf_encryption))
$(eval $(call gb_CppunitTest_use_ure,vcl_pdf_encryption))
$(eval $(call gb_CppunitTest_use_vcl,vcl_pdf_encryption))
$(eval $(call gb_CppunitTest_use_rdb,vcl_pdf_encryption,services))
$(eval $(call gb_CppunitTest_use_configuration,vcl_pdf_encryption))

# vim: set noet sw=4 ts=4:
