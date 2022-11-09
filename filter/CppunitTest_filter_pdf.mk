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

$(eval $(call gb_CppunitTest_CppunitTest,filter_pdf))

$(eval $(call gb_CppunitTest_use_externals,filter_pdf,\
	boost_headers \
))

$(eval $(call gb_CppunitTest_add_exception_objects,filter_pdf, \
    filter/qa/pdf \
))

$(eval $(call gb_CppunitTest_use_libraries,filter_pdf, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    subsequenttest \
    test \
    tl \
    unotest \
    utl \
    vcl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,filter_pdf))

$(eval $(call gb_CppunitTest_use_ure,filter_pdf))
$(eval $(call gb_CppunitTest_use_vcl,filter_pdf))

$(eval $(call gb_CppunitTest_use_rdb,filter_pdf,services))

$(eval $(call gb_CppunitTest_use_custom_headers,filter_pdf,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,filter_pdf))

# vim: set noet sw=4 ts=4:
