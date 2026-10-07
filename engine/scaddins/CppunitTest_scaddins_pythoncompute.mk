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

$(eval $(call gb_CppunitTest_CppunitTest,scaddins_pythoncompute))

$(eval $(call gb_CppunitTest_add_exception_objects,scaddins_pythoncompute, \
    scaddins/qa/pythoncompute \
))

$(eval $(call gb_CppunitTest_set_include,scaddins_pythoncompute,\
    $$(INCLUDE) \
    -I$(SRCDIR)/scaddins/source/pythoncompute \
))

$(eval $(call gb_CppunitTest_use_libraries,scaddins_pythoncompute, \
    comphelper \
    cppu \
    cppuhelper \
    pythoncompute \
    sal \
    test \
    unotest \
    vcl \
))

$(eval $(call gb_CppunitTest_use_externals,scaddins_pythoncompute, \
    boost_headers \
))

$(eval $(call gb_CppunitTest_use_ure,scaddins_pythoncompute))
$(eval $(call gb_CppunitTest_use_vcl,scaddins_pythoncompute))

$(eval $(call gb_CppunitTest_use_rdb,scaddins_pythoncompute,services))

$(eval $(call gb_CppunitTest_use_configuration,scaddins_pythoncompute))

$(eval $(call gb_CppunitTest_use_sdk_api,scaddins_pythoncompute))

$(eval $(call gb_CppunitTest_use_internal_comprehensive_api,scaddins_pythoncompute,\
    scaddins \
))

# vim: set noet sw=4 ts=4:
