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

$(eval $(call gb_CppunitTest_CppunitTest,oox_testscene3d))

$(eval $(call gb_CppunitTest_use_externals,oox_testscene3d,\
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_add_exception_objects,oox_testscene3d, \
    oox/qa/unit/testscene3d \
))

$(eval $(call gb_CppunitTest_use_libraries,oox_testscene3d, \
    comphelper \
    cppu \
    cppuhelper \
    oox \
    sal \
    sfx \
    subsequenttest \
    svx \
    svxcore \
    test \
    tl \
    unotest \
    utl \
    vcl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,oox_testscene3d))

$(eval $(call gb_CppunitTest_use_ure,oox_testscene3d))
$(eval $(call gb_CppunitTest_use_vcl,oox_testscene3d))

$(eval $(call gb_CppunitTest_use_rdb,oox_testscene3d,services))

$(eval $(call gb_CppunitTest_use_custom_headers,oox_testscene3d,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,oox_testscene3d))

$(eval $(call gb_CppunitTest_add_arguments,oox_testscene3d, \
    -env:arg-env=$(gb_Helper_LIBRARY_PATH_VAR)"$$$${$(gb_Helper_LIBRARY_PATH_VAR)+=$$$$$(gb_Helper_LIBRARY_PATH_VAR)}" \
))

# vim: set noet sw=4 ts=4:
