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

$(eval $(call gb_CppunitTest_CppunitTest,sw_txtimport))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sw_txtimport))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_txtimport, \
    sw/qa/extras/txtimport/txtimport \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_txtimport, \
    comphelper \
    cppu \
    cppuhelper \
    i18nlangtag \
    sal \
    sfx \
    subsequenttest \
    sw \
	swqahelper \
    test \
    tl \
    unotest \
    utl \
    vcl \
    $(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_externals,sw_txtimport,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_txtimport,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/source/uibase/inc \
    -I$(SRCDIR)/sw/qa/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_txtimport,\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_txtimport))
$(eval $(call gb_CppunitTest_use_vcl,sw_txtimport))

$(eval $(call gb_CppunitTest_use_rdb,sw_txtimport,services))

$(eval $(call gb_CppunitTest_use_configuration,sw_txtimport))

ifneq ($(filter MORE_FONTS,$(BUILD_TYPE)),)
# Note: This test intentionally sets non-application font use to 'deny',
# rather than 'abort'. This temporarily works around an issue causing
# these tests to fail on fontconfig systems with CJK fonts installed.
#
# See tdf#161533
$(eval $(call gb_CppunitTest_set_non_application_font_use,sw_txtimport,deny))
endif

# vim: set noet sw=4 ts=4:
