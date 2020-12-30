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
    vcl/qa/cppunit/GraphicFormatDetectorTest \
    vcl/qa/cppunit/GraphicNativeMetadataTest \
    vcl/qa/cppunit/VectorGraphicSearchTest \
    vcl/qa/cppunit/BinaryDataContainerTest \
))

$(eval $(call gb_CppunitTest_use_externals,vcl_graphic_test, \
    boost_headers \
    $(if $(filter PDFIUM,$(BUILD_TYPE)),pdfium) \
))
ifeq ($(TLS),NSS)
$(eval $(call gb_CppunitTest_use_externals,vcl_graphic_test,\
       plc4 \
       nss3 \
))
endif

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
$(eval $(call gb_CppunitTest_use_rdb,vcl_graphic_test,services))
$(eval $(call gb_CppunitTest_use_configuration,vcl_graphic_test))

# we need to explicitly depend on Library_gie because it's dynamically loaded for .gif
$(call gb_CppunitTest_get_target,vcl_graphic_test) : $(call gb_Library_get_target,gie)

# vim: set noet sw=4 ts=4:
