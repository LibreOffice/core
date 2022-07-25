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

$(eval $(call gb_CppunitTest_CppunitTest,sw_filter_ww8))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sw_filter_ww8))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_filter_ww8, \
    sw/qa/filter/ww8/ww8 \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_filter_ww8, \
    comphelper \
    cppu \
    cppuhelper \
    editeng \
    sal \
    sfx \
    svl \
    svx \
    svxcore \
    sw \
    swqahelper \
    test \
    unotest \
    utl \
    vcl \
    tl \
))

$(eval $(call gb_CppunitTest_use_externals,sw_filter_ww8,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_filter_ww8,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/source/uibase/inc \
    -I$(SRCDIR)/sw/qa/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_filter_ww8,\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_filter_ww8))
$(eval $(call gb_CppunitTest_use_vcl,sw_filter_ww8))

$(eval $(call gb_CppunitTest_use_rdb,sw_filter_ww8,services))

$(eval $(call gb_CppunitTest_use_custom_headers,sw_filter_ww8,\
    officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,sw_filter_ww8))

$(eval $(call gb_CppunitTest_use_uiconfigs,sw_filter_ww8, \
    modules/swriter \
))

$(eval $(call gb_CppunitTest_use_more_fonts,sw_filter_ww8))

# vim: set noet sw=4 ts=4:
