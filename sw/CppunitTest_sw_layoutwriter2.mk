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

$(eval $(call gb_CppunitTest_CppunitTest,sw_layoutwriter2))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sw_layoutwriter2))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_layoutwriter2, \
    sw/qa/extras/layout/layout2 \
))

# note: this links msword only for the reason to have an order dependency,
# because "make sw.check" will not see the dependency through services.rdb
$(eval $(call gb_CppunitTest_use_libraries,sw_layoutwriter2, \
    comphelper \
    cppu \
    cppuhelper \
    editeng \
    msword \
    sal \
    sfx \
    svl \
    svt \
    svxcore \
    sw \
    test \
    unotest \
    vcl \
    tl \
    utl \
    i18nlangtag \
))

$(eval $(call gb_CppunitTest_use_externals,sw_layoutwriter2,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_layoutwriter2,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/source/uibase/inc \
    -I$(SRCDIR)/sw/qa/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_layoutwriter2,\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_layoutwriter2))
$(eval $(call gb_CppunitTest_use_vcl,sw_layoutwriter2))

$(eval $(call gb_CppunitTest_use_rdb,sw_layoutwriter2,services))

$(eval $(call gb_CppunitTest_use_custom_headers,sw_layoutwriter2,\
    officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,sw_layoutwriter2))

$(eval $(call gb_CppunitTest_use_uiconfigs,sw_layoutwriter2, \
    modules/swriter \
))

$(call gb_CppunitTest_get_target,sw_layoutwriter2): \
    $(call gb_Library_get_target,textconv_dict)

$(eval $(call gb_CppunitTest_use_more_fonts,sw_layoutwriter2))

# vim: set noet sw=4 ts=4:
