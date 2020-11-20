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

$(eval $(call gb_CppunitTest_CppunitTest,sd_filter_eppt))

$(eval $(call gb_CppunitTest_use_externals,sd_filter_eppt,\
	boost_headers \
))

$(eval $(call gb_CppunitTest_add_exception_objects,sd_filter_eppt, \
    sd/qa/filter/eppt/eppt \
))

$(eval $(call gb_CppunitTest_use_libraries,sd_filter_eppt, \
    comphelper \
    cppu \
    sdfilt \
    sal \
    test \
    unotest \
    utl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sd_filter_eppt))

$(eval $(call gb_CppunitTest_use_ure,sd_filter_eppt))
$(eval $(call gb_CppunitTest_use_vcl,sd_filter_eppt))

$(eval $(call gb_CppunitTest_use_rdb,sd_filter_eppt,services))

$(eval $(call gb_CppunitTest_use_custom_headers,sd_filter_eppt,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,sd_filter_eppt))

# vim: set noet sw=4 ts=4:
