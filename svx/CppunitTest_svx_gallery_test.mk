# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#*************************************************************************

$(eval $(call gb_CppunitTest_CppunitScreenShot,svx_gallery_test))

$(eval $(call gb_CppunitTest_add_exception_objects,svx_gallery_test, \
    svx/qa/unit/gallery/test_gallery \
))

$(eval $(call gb_CppunitTest_set_include,svx_gallery_test,\
    -I$(SRCDIR)/svx/source/inc \
    -I$(SRCDIR)/svx/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_libraries,svx_gallery_test, \
	sal \
	utl \
	svxcore \
	comphelper \
	tl \
	svl \
	sfx \
	unotest \
	test \
))

$(eval $(call gb_CppunitTest_use_externals,svx_gallery_test,boost_headers))

$(eval $(call gb_CppunitTest_use_vcl,svx_gallery_test))

$(eval $(call gb_CppunitTest_use_sdk_api,svx_gallery_test))

$(eval $(call gb_CppunitTest_use_configuration,svx_gallery_test))

$(eval $(call gb_CppunitTest_use_rdb,svx_gallery_test,services))

$(eval $(call gb_CppunitTest_use_ure,svx_gallery_test))









# vim: set noet sw=4 ts=4:
