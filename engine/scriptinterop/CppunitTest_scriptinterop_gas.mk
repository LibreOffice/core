# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# Copyright the Collabora Office contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,scriptinterop_gas))

$(eval $(call gb_CppunitTest_add_exception_objects,scriptinterop_gas, \
    scriptinterop/qa/unit/gas \
))

$(eval $(call gb_CppunitTest_use_api,scriptinterop_gas, \
    scriptinterop \
))

$(eval $(call gb_CppunitTest_use_configuration,scriptinterop_gas))

$(eval $(call gb_CppunitTest_use_libraries,scriptinterop_gas, \
    comphelper \
    cppu \
    cppuhelper \
    jsuno \
    sal \
    subsequenttest \
    test \
    tl \
    unotest \
    utl \
))

$(eval $(call gb_CppunitTest_use_rdb,scriptinterop_gas,services))

$(eval $(call gb_CppunitTest_use_sdk_api,scriptinterop_gas))

$(eval $(call gb_CppunitTest_use_ure,scriptinterop_gas))

$(eval $(call gb_CppunitTest_use_vcl,scriptinterop_gas))

# vim: set noet sw=4 ts=4:
