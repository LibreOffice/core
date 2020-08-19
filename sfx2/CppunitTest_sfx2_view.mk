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

$(eval $(call gb_CppunitTest_CppunitTest,sfx2_view))

$(eval $(call gb_CppunitTest_use_externals,sfx2_view,\
	boost_headers \
))

$(eval $(call gb_CppunitTest_add_exception_objects,sfx2_view, \
    sfx2/qa/cppunit/view \
))

$(eval $(call gb_CppunitTest_use_libraries,sfx2_view, \
    comphelper \
    cppu \
    sal \
    test \
    unotest \
    sfx \
    svl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sfx2_view))

$(eval $(call gb_CppunitTest_use_ure,sfx2_view))
$(eval $(call gb_CppunitTest_use_vcl,sfx2_view))

$(eval $(call gb_CppunitTest_use_rdb,sfx2_view,services))

$(eval $(call gb_CppunitTest_use_custom_headers,sfx2_view,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,sfx2_view))

# vim: set noet sw=4 ts=4:
