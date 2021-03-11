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

$(eval $(call gb_CppunitTest_CppunitTest,xmloff_style))

$(eval $(call gb_CppunitTest_use_externals,xmloff_style,\
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_add_exception_objects,xmloff_style, \
    xmloff/qa/unit/style \
))

$(eval $(call gb_CppunitTest_use_libraries,xmloff_style, \
    comphelper \
    cppu \
    embobj \
    sal \
    test \
    unotest \
    utl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,xmloff_style))

$(eval $(call gb_CppunitTest_use_ure,xmloff_style))
$(eval $(call gb_CppunitTest_use_vcl,xmloff_style))

$(eval $(call gb_CppunitTest_use_rdb,xmloff_style,services))

$(eval $(call gb_CppunitTest_use_custom_headers,xmloff_style,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,xmloff_style))

# vim: set noet sw=4 ts=4:
