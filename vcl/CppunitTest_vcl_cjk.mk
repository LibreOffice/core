# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,vcl_cjk))

$(eval $(call gb_CppunitTest_set_include,vcl_cjk,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_CppunitTest_add_exception_objects,vcl_cjk, \
	vcl/qa/cppunit/cjktext \
))

$(eval $(call gb_CppunitTest_use_externals,vcl_cjk,\
	boost_headers \
	harfbuzz \
))

ifeq ($(SYSTEM_ICU),TRUE)
$(eval $(call gb_CppunitTest_use_externals,vcl_cjk,\
	icuuc \
))
else
$(eval $(call gb_CppunitTest_use_externals,vcl_cjk,\
        icu_headers \
))
endif

$(eval $(call gb_CppunitTest_use_libraries,vcl_cjk, \
	comphelper \
	cppu \
	cppuhelper \
	i18nlangtag \
	sal \
	svt \
	test \
	tl \
	unotest \
	vcl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,vcl_cjk))

$(eval $(call gb_CppunitTest_use_ure,vcl_cjk))
$(eval $(call gb_CppunitTest_use_vcl,vcl_cjk))

$(eval $(call gb_CppunitTest_use_components,vcl_cjk,\
	configmgr/source/configmgr \
	i18npool/util/i18npool \
	ucb/source/core/ucb1 \
))

$(eval $(call gb_CppunitTest_use_configuration,vcl_cjk))

$(eval $(call gb_CppunitTest_use_more_fonts,vcl_cjk))

# we don't have any bundled cjk fonts, so allow use of
# system fonts for the cjk tests, tests have to survive
# unavailable fonts
$(eval $(call gb_CppunitTest_set_non_application_font_use,vcl_cjk,allow))

# vim: set noet sw=4 ts=4:
