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

$(eval $(call gb_CppunitTest_CppunitTest,sc_ucalc))

$(eval $(call gb_CppunitTest_add_exception_objects,sc_ucalc, \
    sc/qa/unit/ucalc \
    sc/qa/unit/ucalc_formula \
    sc/qa/unit/ucalc_pivottable \
    sc/qa/unit/ucalc_sharedformula \
))

$(eval $(call gb_CppunitTest_use_library_objects,sc_ucalc, \
	sc \
	scqahelper \
))

ifeq ($(ENABLE_TELEPATHY),TRUE)
$(eval $(call gb_CppunitTest_use_libraries,sc_ucalc,tubes))
endif

$(eval $(call gb_CppunitTest_use_externals,sc_ucalc,\
	boost_headers \
    icu_headers \
    icudata \
    icui18n \
    icuuc \
	libxml2 \
	mdds_headers \
	orcus \
	orcus-parser \
))

$(eval $(call gb_CppunitTest_use_libraries,sc_ucalc, \
    avmedia \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    drawinglayer \
    editeng \
    for \
    forui \
    i18nlangtag \
    sal \
    salhelper \
    sax \
    sb \
    sfx \
    sot \
    svl \
    svt \
    svx \
    svxcore \
	test \
    tk \
    tl \
    ucbhelper \
	unotest \
    utl \
    vbahelper \
    vcl \
    xo \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_set_include,sc_ucalc,\
    -I$(SRCDIR)/sc/source/ui/inc \
    -I$(SRCDIR)/sc/source/core/inc \
    -I$(SRCDIR)/sc/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sc_ucalc,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,sc_ucalc))

$(eval $(call gb_CppunitTest_use_components,sc_ucalc,\
    configmgr/source/configmgr \
    fileaccess/source/fileacc \
    framework/util/fwk \
    i18npool/util/i18npool \
    i18npool/source/search/i18nsearch \
    sax/source/expatwrap/expwrap \
    sfx2/util/sfx \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    unoxml/source/service/unoxml \
    uui/util/uui \
))

$(eval $(call gb_CppunitTest_use_configuration,sc_ucalc))

# vim: set noet sw=4 ts=4:
