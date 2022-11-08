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

# template for layoutwriter tests (there are several so that they can be run in parallel)
define sw_layoutwriter_test

$(eval $(call gb_CppunitTest_CppunitTest,sw_layoutwriter$(1)))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sw_layoutwriter$(1)))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_layoutwriter$(1), \
    sw/qa/extras/layout/layout$(1) \
))

# note: this links msword only for the reason to have an order dependency,
# because "make sw.check" will not see the dependency through services.rdb
$(eval $(call gb_CppunitTest_use_libraries,sw_layoutwriter$(1), \
    comphelper \
    cppu \
    cppuhelper \
    editeng \
    msword \
    sal \
    sfx \
    subsequenttest \
    svl \
    svt \
    svxcore \
    sw \
	swqahelper \
    test \
    unotest \
    vcl \
    tl \
    utl \
    i18nlangtag \
))

$(eval $(call gb_CppunitTest_use_externals,sw_layoutwriter$(1),\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_layoutwriter$(1),\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/source/uibase/inc \
    -I$(SRCDIR)/sw/qa/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_layoutwriter$(1),\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_layoutwriter$(1)))
$(eval $(call gb_CppunitTest_use_vcl,sw_layoutwriter$(1)))

$(eval $(call gb_CppunitTest_use_rdb,sw_layoutwriter$(1),services))

$(eval $(call gb_CppunitTest_use_custom_headers,sw_layoutwriter$(1),\
    officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,sw_layoutwriter$(1)))

$(eval $(call gb_CppunitTest_use_uiconfigs,sw_layoutwriter$(1), \
    modules/swriter \
    sfx \
    svt \
))

$(call gb_CppunitTest_get_target,sw_layoutwriter$(1)): \
    $(call gb_Library_get_target,textconv_dict)

# assert if font/glyph fallback occurs
$(eval $(call gb_CppunitTest_set_non_application_font_use,sw_layoutwriter$(1),abort))

$(eval $(call gb_CppunitTest_use_more_fonts,sw_layoutwriter$(1)))

endef

# vim: set noet sw=4 ts=4:
