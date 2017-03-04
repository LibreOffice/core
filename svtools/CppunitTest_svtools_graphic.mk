# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,svtools_graphic))

$(eval $(call gb_CppunitTest_use_externals,svtools_graphic,\
	boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_use_sdk_api,svtools_graphic))

$(eval $(call gb_CppunitTest_use_ure,svtools_graphic))

$(eval $(call gb_CppunitTest_use_vcl,svtools_graphic))

$(eval $(call gb_CppunitTest_use_libraries,svtools_graphic, \
	comphelper \
	cppu \
	cppuhelper \
	tl \
	sal \
	svt \
	sw \
	test \
	unotest \
	vcl \
))

$(eval $(call gb_CppunitTest_set_include,svtools_graphic,\
    -I$(SRCDIR)/sw/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_custom_headers,svtools_graphic,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,svtools_graphic))

$(eval $(call gb_CppunitTest_use_rdb,svtools_graphic,services))

$(eval $(call gb_CppunitTest_add_exception_objects,svtools_graphic, \
    svtools/qa/unit/GraphicObjectTest \
))

# vim: set noet sw=4 ts=4:
