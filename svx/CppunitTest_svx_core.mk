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

$(eval $(call gb_CppunitTest_CppunitTest,svx_core))

$(eval $(call gb_CppunitTest_use_externals,svx_core,\
	boost_headers \
))

$(eval $(call gb_CppunitTest_add_exception_objects,svx_core, \
    svx/qa/unit/core \
))

$(eval $(call gb_CppunitTest_use_libraries,svx_core, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    svx \
    svxcore \
    test \
    tl \
    unotest \
    utl \
    vcl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,svx_core))

$(eval $(call gb_CppunitTest_use_ure,svx_core))
$(eval $(call gb_CppunitTest_use_vcl,svx_core))

$(eval $(call gb_CppunitTest_use_rdb,svx_core,services))

$(eval $(call gb_CppunitTest_use_custom_headers,svx_core,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,svx_core))

# vim: set noet sw=4 ts=4:
