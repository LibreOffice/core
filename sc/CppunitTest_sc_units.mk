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

$(eval $(call gb_CppunitTest_CppunitTest,sc_units))

$(eval $(call gb_CppunitTest_add_exception_objects,sc_units, \
    sc/qa/unit/units \
))

$(eval $(call gb_CppunitTest_use_library_objects,sc_units, \
	sc \
	scqahelper \
))

ifeq ($(ENABLE_TELEPATHY),TRUE)
$(eval $(call gb_CppunitTest_use_libraries,sc_units,tubes))
endif

$(eval $(call gb_CppunitTest_use_externals,sc_units,\
	boost_headers \
    icu_headers \
    icui18n \
    icuuc \
	libxml2 \
	mdds_headers \
	orcus \
	orcus-parser \
    $(if $(ENABLE_CALC_UNITVERIFICATION), \
		udunits2 \
    ) \
))

$(eval $(call gb_CppunitTest_use_libraries,sc_units, \
	$(call gb_Helper_optional,AVMEDIA,avmedia) \
    basegfx \
    $(call gb_Helper_optional,OPENCL, \
        clew) \
	comphelper \
	cppu \
    cppuhelper \
    drawinglayer \
    editeng \
    for \
    forui \
    i18nlangtag \
    i18nutil \
	$(call gb_Helper_optional,OPENCL, \
		opencl) \
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

$(eval $(call gb_CppunitTest_set_include,sc_units,\
    -I$(SRCDIR)/sc/source/ui/inc \
    -I$(SRCDIR)/sc/source/core/inc \
    -I$(SRCDIR)/sc/inc \
	-I$(SRCDIR)/sc/source/core/units \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sc_units,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,sc_units))
$(eval $(call gb_CppunitTest_use_vcl,sc_units))

$(eval $(call gb_CppunitTest_use_components,sc_units,\
    configmgr/source/configmgr \
    framework/util/fwk \
    i18npool/source/search/i18nsearch \
    i18npool/util/i18npool \
    sax/source/expatwrap/expwrap \
    scaddins/source/analysis/analysis \
    scaddins/source/datefunc/date \
    scaddins/source/pricing/pricing \
    sfx2/util/sfx \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    unoxml/source/service/unoxml \
    uui/util/uui \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_CppunitTest_add_libs,sc_units,\
    -ldl \
	-lrt \
))
endif

$(eval $(call gb_CppunitTest_use_configuration,sc_units))

# vim: set noet sw=4 ts=4:
