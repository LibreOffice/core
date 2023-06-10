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

$(eval $(call gb_CppunitTest_CppunitTest,oox_mcgr))

$(eval $(call gb_CppunitTest_use_externals,oox_mcgr,\
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_add_exception_objects,oox_mcgr, \
    oox/qa/unit/mcgr \
))

$(eval $(call gb_CppunitTest_use_libraries,oox_mcgr, \
    comphelper \
    cppu \
    cppuhelper \
    oox \
    sal \
    subsequenttest \
    test \
    unotest \
    utl \
    tl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,oox_mcgr))

$(eval $(call gb_CppunitTest_use_ure,oox_mcgr))
$(eval $(call gb_CppunitTest_use_vcl,oox_mcgr))

$(eval $(call gb_CppunitTest_use_rdb,oox_mcgr,services))

$(eval $(call gb_CppunitTest_use_custom_headers,oox_mcgr,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,oox_mcgr))

$(eval $(call gb_CppunitTest_add_arguments,oox_mcgr, \
    -env:arg-env=$(gb_Helper_LIBRARY_PATH_VAR)"$$$${$(gb_Helper_LIBRARY_PATH_VAR)+=$$$$$(gb_Helper_LIBRARY_PATH_VAR)}" \
))

# vim: set noet sw=4 ts=4:
