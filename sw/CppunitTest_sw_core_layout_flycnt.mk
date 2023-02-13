# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#*************************************************************************

$(eval $(call gb_CppunitTest_CppunitTest,sw_core_layout_flycnt))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sw_core_layout_flycnt))

# TODO merge this with sw_core_layout once SwFormatFlySplit::SetForce() is gone.
$(eval $(call gb_CppunitTest_add_exception_objects,sw_core_layout_flycnt, \
    sw/qa/core/layout/flycnt \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_core_layout_flycnt, \
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

$(eval $(call gb_CppunitTest_use_externals,sw_core_layout_flycnt,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_core_layout_flycnt,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/source/uibase/inc \
    -I$(SRCDIR)/sw/qa/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_core_layout_flycnt,\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_core_layout_flycnt))
$(eval $(call gb_CppunitTest_use_vcl,sw_core_layout_flycnt))

$(eval $(call gb_CppunitTest_use_rdb,sw_core_layout_flycnt,services))

$(eval $(call gb_CppunitTest_use_custom_headers,sw_core_layout_flycnt,\
    officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,sw_core_layout_flycnt))

$(eval $(call gb_CppunitTest_use_uiconfigs,sw_core_layout_flycnt, \
    modules/swriter \
    svt \
    svx \
))

# assert if font/glyph fallback occurs
$(eval $(call gb_CppunitTest_set_non_application_font_use,sw_core_layout_flycnt,abort))

$(eval $(call gb_CppunitTest_use_more_fonts,sw_core_layout_flycnt))

# vim: set noet sw=4 ts=4:
