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

$(eval $(call gb_CppunitTest_CppunitTest,xmloff_text))

$(eval $(call gb_CppunitTest_use_externals,xmloff_text,\
	boost_headers \
))

$(eval $(call gb_CppunitTest_add_exception_objects,xmloff_text, \
    xmloff/qa/unit/text \
))

$(eval $(call gb_CppunitTest_use_libraries,xmloff_text, \
    comphelper \
    cppu \
    embobj \
    sal \
    test \
    unotest \
    utl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,xmloff_text))

$(eval $(call gb_CppunitTest_use_ure,xmloff_text))
$(eval $(call gb_CppunitTest_use_vcl,xmloff_text))

$(eval $(call gb_CppunitTest_use_rdb,xmloff_text,services))

$(eval $(call gb_CppunitTest_use_custom_headers,xmloff_text,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,xmloff_text))

# vim: set noet sw=4 ts=4:
