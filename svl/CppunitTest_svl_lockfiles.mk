# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,svl_lockfiles))

$(eval $(call gb_CppunitTest_use_sdk_api,svl_lockfiles))

$(eval $(call gb_CppunitTest_use_api,svl_lockfiles,\
    udkapi \
    offapi \
    oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,svl_lockfiles))

$(eval $(call gb_CppunitTest_use_vcl,svl_lockfiles))


$(eval $(call gb_CppunitTest_add_exception_objects,svl_lockfiles, \
        svl/qa/unit/lockfiles/test_lockfiles \
))

$(eval $(call gb_CppunitTest_use_libraries,svl_lockfiles, \
    comphelper \
    cppu \
    cppuhelper \
    tl \
    sal \
    svl \
    svt \
    sw \
    test \
    unotest \
    utl \
    vcl \
))

$(eval $(call gb_CppunitTest_use_rdb,svl_lockfiles,services))

$(eval $(call gb_CppunitTest_use_custom_headers,svl_lockfiles,\
        officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,svl_lockfiles))

# vim: set noet sw=4 ts=4:
