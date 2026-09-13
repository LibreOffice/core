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

$(eval $(call gb_CppunitTest_CppunitTest,lingucomponent_lightproof))

$(eval $(call gb_CppunitTest_add_exception_objects,lingucomponent_lightproof, \
    lingucomponent/qa/unit/LightproofTest \
))

$(eval $(call gb_CppunitTest_use_libraries,lingucomponent_lightproof, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    test \
    unotest \
    utl \
))

$(eval $(call gb_CppunitTest_use_api,lingucomponent_lightproof,\
    udkapi \
    offapi \
))

# The checker reads its rule files from the install tree.
$(eval $(call gb_CppunitTest_use_package,lingucomponent_lightproof,lingucomponent_lightproof))

$(eval $(call gb_CppunitTest_use_configuration,lingucomponent_lightproof))

$(eval $(call gb_CppunitTest_use_ure,lingucomponent_lightproof))

$(eval $(call gb_CppunitTest_use_rdb,lingucomponent_lightproof,services))

# The test checks the Russian rule package the dictionaries module compiles.
$(call gb_CppunitTest_get_target,lingucomponent_lightproof) : \
    $(call gb_CustomTarget_get_target,dictionaries/lightproof)

# vim: set noet sw=4 ts=4:
