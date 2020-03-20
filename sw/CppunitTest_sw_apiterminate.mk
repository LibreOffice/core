# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sw_apiterminate))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_apiterminate, \
    sw/qa/api/terminate \
))

$(eval $(call gb_CppunitTest_use_api,sw_apiterminate, \
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sw_apiterminate))

$(eval $(call gb_CppunitTest_use_configuration,sw_apiterminate))

$(eval $(call gb_CppunitTest_use_libraries,sw_apiterminate, \
    comphelper \
    cppu \
    sal \
    subsequenttest \
    test \
    unotest \
))

$(eval $(call gb_CppunitTest_use_rdb,sw_apiterminate,services))

$(eval $(call gb_CppunitTest_use_vcl,sw_apiterminate))

$(eval $(call gb_CppunitTest_use_uiconfigs,sw_apiterminate, \
    svx \
))

$(eval $(call gb_CppunitTest_use_ure,sw_apiterminate))

# vim: set noet sw=4 ts=4:
