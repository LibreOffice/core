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

$(eval $(call gb_CppunitTest_CppunitTest,filter_svg))

$(eval $(call gb_CppunitTest_use_externals,filter_svg,\
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_add_exception_objects,filter_svg, \
    filter/qa/unit/svg \
))

$(eval $(call gb_CppunitTest_use_libraries,filter_svg, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    test \
    unotest \
    utl \
    tl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,filter_svg))

$(eval $(call gb_CppunitTest_use_ure,filter_svg))
$(eval $(call gb_CppunitTest_use_vcl,filter_svg))

$(eval $(call gb_CppunitTest_use_rdb,filter_svg,services))

$(eval $(call gb_CppunitTest_use_custom_headers,filter_svg,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,filter_svg))

# vim: set noet sw=4 ts=4:
