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

$(eval $(call gb_CppunitTest_CppunitTest,sc_oox_helpers_test))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sc_oox_helpers_test))

$(eval $(call gb_CppunitTest_add_exception_objects,sc_oox_helpers_test, \
    sc/qa/unit/RedundantParenthesesStripperTest \
))

$(eval $(call gb_CppunitTest_use_externals,sc_oox_helpers_test, \
    boost_headers \
    mdds_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_use_libraries,sc_oox_helpers_test, \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    docmodel \
    sal \
    salhelper \
    sax \
    sc \
    scfilt \
    scqahelper \
    sfx \
    subsequenttest \
    test \
    tl \
    unotest \
    utl \
    vcl \
))

$(eval $(call gb_CppunitTest_set_include,sc_oox_helpers_test,\
    -I$(SRCDIR)/sc/source/ui/inc \
    -I$(SRCDIR)/sc/source/filter/inc \
    -I$(SRCDIR)/sc/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sc_oox_helpers_test,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sc_oox_helpers_test))

$(eval $(call gb_CppunitTest_use_ure,sc_oox_helpers_test))
$(eval $(call gb_CppunitTest_use_vcl,sc_oox_helpers_test))

$(eval $(call gb_CppunitTest_use_rdb,sc_oox_helpers_test,services))

$(eval $(call gb_CppunitTest_use_components,sc_oox_helpers_test))

$(eval $(call gb_CppunitTest_use_configuration,sc_oox_helpers_test))

$(eval $(call gb_CppunitTest_add_arguments,sc_oox_helpers_test, \
    -env:arg-env=$(gb_Helper_LIBRARY_PATH_VAR)"$$$${$(gb_Helper_LIBRARY_PATH_VAR)+=$$$$$(gb_Helper_LIBRARY_PATH_VAR)}" \
))

# vim: set noet sw=4 ts=4:
