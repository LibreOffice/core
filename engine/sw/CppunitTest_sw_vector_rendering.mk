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

$(eval $(call gb_CppunitTest_CppunitTest,sw_vector_rendering))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sw_vector_rendering))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_vector_rendering, \
    sw/qa/unit/vector/VectorRenderingTest \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_vector_rendering, \
    basegfx \
    drawinglayer \
    drawinglayercore \
    editeng \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    sfx \
    subsequenttest \
    sw \
    swqahelper \
    test \
    unotest \
    utl \
    vcl \
    svt \
    tl \
    svl \
    svxcore \
))

$(eval $(call gb_CppunitTest_use_externals,sw_vector_rendering,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_vector_rendering,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/source/uibase/inc \
    -I$(SRCDIR)/sw/qa/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_vector_rendering,\
    udkapi \
    offapi \
    oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_vector_rendering))
$(eval $(call gb_CppunitTest_use_vcl,sw_vector_rendering))

$(eval $(call gb_CppunitTest_use_rdb,sw_vector_rendering,services))

$(eval $(call gb_CppunitTest_use_custom_headers,sw_vector_rendering,\
    officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,sw_vector_rendering))

$(eval $(call gb_CppunitTest_use_uiconfigs,sw_vector_rendering, \
    modules/swriter \
    sfx \
    svt \
    svx \
))

# assert if font/glyph fallback occurs
$(eval $(call gb_CppunitTest_set_non_application_font_use,sw_vector_rendering,abort))

$(eval $(call gb_CppunitTest_use_more_fonts,sw_vector_rendering))

# vim: set noet sw=4 ts=4:
