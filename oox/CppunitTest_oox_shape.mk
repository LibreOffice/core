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

$(eval $(call gb_CppunitTest_CppunitTest,oox_shape))

$(eval $(call gb_CppunitTest_use_externals,oox_shape,\
	boost_headers \
))

$(eval $(call gb_CppunitTest_add_exception_objects,oox_shape, \
    oox/qa/unit/shape \
))

$(eval $(call gb_CppunitTest_use_libraries,oox_shape, \
    comphelper \
    cppu \
    oox \
    sal \
    svx \
    svxcore \
    tl \
    test \
    unotest \
    utl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,oox_shape))

$(eval $(call gb_CppunitTest_use_ure,oox_shape))
$(eval $(call gb_CppunitTest_use_vcl,oox_shape))

$(eval $(call gb_CppunitTest_use_rdb,oox_shape,services))

$(eval $(call gb_CppunitTest_use_custom_headers,oox_shape,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,oox_shape))

# vim: set noet sw=4 ts=4:
