# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,filter_pdfimporter_test))

$(eval $(call gb_CppunitTest_use_externals,filter_pdfimporter_test,\
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_add_exception_objects,filter_pdfimporter_test, \
    filter/qa/cppunit/PdfImporterTest \
))

$(eval $(call gb_CppunitTest_use_library_objects,filter_pdfimporter_test,\
    pdfimporter \
))

$(eval $(call gb_CppunitTest_use_libraries,filter_pdfimporter_test, \
    basegfx \
    drawinglayer \
    drawinglayercore \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    salhelper \
    test \
    unotest \
    utl \
    tl \
    vcl \
    tk \
))

$(eval $(call gb_CppunitTest_use_configuration,filter_pdfimporter_test))
$(eval $(call gb_CppunitTest_use_sdk_api,filter_pdfimporter_test))
$(eval $(call gb_CppunitTest_use_ure,filter_pdfimporter_test))
$(eval $(call gb_CppunitTest_use_vcl,filter_pdfimporter_test))
$(eval $(call gb_CppunitTest_use_rdb,filter_pdfimporter_test,services))


# vim: set noet sw=4 ts=4:
