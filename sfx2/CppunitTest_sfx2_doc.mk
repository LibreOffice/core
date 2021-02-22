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

$(eval $(call gb_CppunitTest_CppunitTest,sfx2_doc))

$(eval $(call gb_CppunitTest_use_externals,sfx2_doc,\
	boost_headers \
))

$(eval $(call gb_CppunitTest_add_exception_objects,sfx2_doc, \
    sfx2/qa/cppunit/doc \
))

$(eval $(call gb_CppunitTest_use_libraries,sfx2_doc, \
    comphelper \
    cppu \
    sal \
    test \
    unotest \
    sfx \
    svl \
    utl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sfx2_doc))

$(eval $(call gb_CppunitTest_use_ure,sfx2_doc))
$(eval $(call gb_CppunitTest_use_vcl,sfx2_doc))

$(eval $(call gb_CppunitTest_use_rdb,sfx2_doc,services))

$(eval $(call gb_CppunitTest_use_custom_headers,sfx2_doc,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,sfx2_doc))

# vim: set noet sw=4 ts=4:
