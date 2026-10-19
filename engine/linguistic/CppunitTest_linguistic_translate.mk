# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,linguistic_translate))

$(eval $(call gb_CppunitTest_add_exception_objects,linguistic_translate, \
    linguistic/qa/translate \
))

$(eval $(call gb_CppunitTest_use_libraries,linguistic_translate, \
    comphelper \
    cppu \
    cppuhelper \
    lng \
    sal \
    test \
    unotest \
    utl \
))

$(eval $(call gb_CppunitTest_use_api,linguistic_translate,\
    udkapi \
    offapi \
))

$(eval $(call gb_CppunitTest_use_configuration,linguistic_translate))

$(eval $(call gb_CppunitTest_use_ure,linguistic_translate))

$(eval $(call gb_CppunitTest_use_rdb,linguistic_translate,services))

$(eval $(call gb_CppunitTest_use_custom_headers,linguistic_translate,\
    officecfg/registry \
))

# vim: set noet sw=4 ts=4:
