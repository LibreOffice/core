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

$(eval $(call gb_CppunitTest_CppunitTest,svx_styles))

$(eval $(call gb_CppunitTest_use_externals,svx_styles,\
	boost_headers \
))

$(eval $(call gb_CppunitTest_add_exception_objects,svx_styles, \
    svx/qa/unit/styles \
))

$(eval $(call gb_CppunitTest_use_libraries,svx_styles, \
    comphelper \
    cppu \
    svx \
    sal \
    test \
    unotest \
))

$(eval $(call gb_CppunitTest_use_sdk_api,svx_styles))

$(eval $(call gb_CppunitTest_use_ure,svx_styles))
$(eval $(call gb_CppunitTest_use_vcl,svx_styles))

$(eval $(call gb_CppunitTest_use_rdb,svx_styles,services))

$(eval $(call gb_CppunitTest_use_custom_headers,svx_styles,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,svx_styles))

# vim: set noet sw=4 ts=4:
