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

$(eval $(call gb_CppunitTest_CppunitTest,i18npool_defaultnumberingprovider))

$(eval $(call gb_CppunitTest_add_exception_objects,i18npool_defaultnumberingprovider, \
    i18npool/qa/cppunit/test_defaultnumberingprovider \
))

$(eval $(call gb_CppunitTest_use_libraries,i18npool_defaultnumberingprovider, \
    comphelper \
    cppu \
    sal \
    test \
    unotest \
    utl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,i18npool_defaultnumberingprovider))

$(eval $(call gb_CppunitTest_use_ure,i18npool_defaultnumberingprovider))
$(eval $(call gb_CppunitTest_use_vcl,i18npool_defaultnumberingprovider))

$(eval $(call gb_CppunitTest_use_rdb,i18npool_defaultnumberingprovider,services))

$(eval $(call gb_CppunitTest_use_custom_headers,i18npool_defaultnumberingprovider,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,i18npool_defaultnumberingprovider))

# vim: set noet sw=4 ts=4:
