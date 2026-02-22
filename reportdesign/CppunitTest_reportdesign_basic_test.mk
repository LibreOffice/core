# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,reportdesign_basic_test))

$(eval $(call gb_CppunitTest_use_externals,reportdesign_basic_test, \
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_add_exception_objects,reportdesign_basic_test, \
    reportdesign/qa/unit/ReportDesignBasicTests \
))

$(eval $(call gb_CppunitTest_set_include,reportdesign_basic_test,\
    -I$(SRCDIR)/reportdesign/source/inc \
    -I$(SRCDIR)/reportdesign/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_libraries,reportdesign_basic_test, \
    comphelper \
    cppu \
    cppuhelper \
    dbaxml \
    dbtools \
    sal \
    subsequenttest \
    utl \
    test \
    tk \
    tl \
    unotest \
    xo \
))

$(eval $(call gb_CppunitTest_use_api,reportdesign_basic_test,\
    offapi \
    oovbaapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,reportdesign_basic_test))
$(eval $(call gb_CppunitTest_use_vcl,reportdesign_basic_test))

$(eval $(call gb_CppunitTest_use_rdb,reportdesign_basic_test,services))

$(eval $(call gb_CppunitTest_use_configuration,reportdesign_basic_test))

$(eval $(call gb_CppunitTest_use_uiconfigs,reportdesign_basic_test, \
    reportdesign \
))

$(eval $(call gb_CppunitTest_use_jars,reportdesign_basic_test, \
    sdbc_hsqldb \
))

# vim: set noet sw=4 ts=4:
