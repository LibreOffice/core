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

$(eval $(call gb_CppunitTest_CppunitTest,sfx2_autoredaction))

$(eval $(call gb_CppunitTest_use_externals,sfx2_autoredaction,\
	boost_headers \
))

$(eval $(call gb_CppunitTest_add_exception_objects,sfx2_autoredaction, \
    sfx2/qa/cppunit/autoredaction \
))

$(eval $(call gb_CppunitTest_use_libraries,sfx2_autoredaction, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    subsequenttest \
    test \
    unotest \
    sfx \
    svl \
    utl \
    tl \
    svx \
    vcl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sfx2_autoredaction))

$(eval $(call gb_CppunitTest_use_ure,sfx2_autoredaction))
$(eval $(call gb_CppunitTest_use_vcl,sfx2_autoredaction))

$(eval $(call gb_CppunitTest_use_rdb,sfx2_autoredaction,services))

$(eval $(call gb_CppunitTest_use_configuration,sfx2_autoredaction))

# vim: set noet sw=4 ts=4:
