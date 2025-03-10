# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,dbaccess_SingleSelectQueryComposer_test))

$(eval $(call gb_CppunitTest_use_external,dbaccess_SingleSelectQueryComposer_test,boost_headers))

$(eval $(call gb_CppunitTest_add_exception_objects,dbaccess_SingleSelectQueryComposer_test, \
    dbaccess/qa/unit/SingleSelectQueryComposer_test \
))

$(eval $(call gb_CppunitTest_use_libraries,dbaccess_SingleSelectQueryComposer_test, \
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

$(eval $(call gb_CppunitTest_use_api,dbaccess_SingleSelectQueryComposer_test,\
    offapi \
    oovbaapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,dbaccess_SingleSelectQueryComposer_test))
$(eval $(call gb_CppunitTest_use_vcl,dbaccess_SingleSelectQueryComposer_test))

$(eval $(call gb_CppunitTest_use_rdb,dbaccess_SingleSelectQueryComposer_test,services))

$(eval $(call gb_CppunitTest_use_configuration,dbaccess_SingleSelectQueryComposer_test))

$(eval $(call gb_CppunitTest_use_uiconfigs,dbaccess_SingleSelectQueryComposer_test, \
    dbaccess \
))

# vim: set noet sw=4 ts=4:
