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

$(eval $(call gb_CppunitTest_CppunitTest,vcl_filter_igif))

$(eval $(call gb_CppunitTest_use_externals,vcl_filter_igif,\
	boost_headers \
))

$(eval $(call gb_CppunitTest_add_exception_objects,vcl_filter_igif, \
    vcl/qa/cppunit/filter/igif/igif \
))

$(eval $(call gb_CppunitTest_use_libraries,vcl_filter_igif, \
    comphelper \
    cppu \
    sal \
    test \
    tl \
    unotest \
    utl \
    vcl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,vcl_filter_igif))

$(eval $(call gb_CppunitTest_use_ure,vcl_filter_igif))
$(eval $(call gb_CppunitTest_use_vcl,vcl_filter_igif))

$(eval $(call gb_CppunitTest_use_rdb,vcl_filter_igif,services))

$(eval $(call gb_CppunitTest_use_custom_headers,vcl_filter_igif,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,vcl_filter_igif))

# vim: set noet sw=4 ts=4:
