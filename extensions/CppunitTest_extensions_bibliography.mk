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

$(eval $(call gb_CppunitTest_CppunitTest,extensions_bibliography))

$(eval $(call gb_CppunitTest_use_externals,extensions_bibliography,\
	boost_headers \
))

$(eval $(call gb_CppunitTest_add_exception_objects,extensions_bibliography, \
    extensions/qa/bibliography/bibliography \
))

$(eval $(call gb_CppunitTest_use_libraries,extensions_bibliography, \
    comphelper \
    cppu \
    sal \
    test \
    unotest \
))

$(eval $(call gb_CppunitTest_use_sdk_api,extensions_bibliography))

$(eval $(call gb_CppunitTest_use_ure,extensions_bibliography))
$(eval $(call gb_CppunitTest_use_vcl,extensions_bibliography))

$(eval $(call gb_CppunitTest_use_rdb,extensions_bibliography,services))

$(eval $(call gb_CppunitTest_use_custom_headers,extensions_bibliography,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,extensions_bibliography))

# vim: set noet sw=4 ts=4:
