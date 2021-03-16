# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,vcl_bitmap_test))

$(eval $(call gb_CppunitTest_add_exception_objects,vcl_bitmap_test, \
    vcl/qa/cppunit/BitmapTest \
    vcl/qa/cppunit/BitmapExTest \
    vcl/qa/cppunit/bitmapcolor \
    vcl/qa/cppunit/ScanlineToolsTest \
    vcl/qa/cppunit/BitmapScaleTest \
    vcl/qa/cppunit/BitmapFilterTest \
    vcl/qa/cppunit/BmpFilterTest \
))

$(eval $(call gb_CppunitTest_use_externals,vcl_bitmap_test,\
	boost_headers \
	glm_headers \
))
ifeq ($(DISABLE_GUI),)
$(eval $(call gb_CppunitTest_use_externals,vcl_bitmap_test,\
     epoxy \
 ))
endif

$(eval $(call gb_CppunitTest_set_include,vcl_bitmap_test,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_CppunitTest_use_libraries,vcl_bitmap_test, \
	basegfx \
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

$(eval $(call gb_CppunitTest_use_sdk_api,vcl_bitmap_test))

$(eval $(call gb_CppunitTest_use_ure,vcl_bitmap_test))
$(eval $(call gb_CppunitTest_use_vcl,vcl_bitmap_test))

$(eval $(call gb_CppunitTest_use_rdb,vcl_bitmap_test,services))

$(eval $(call gb_CppunitTest_use_configuration,vcl_bitmap_test))

# vim: set noet sw=4 ts=4:
