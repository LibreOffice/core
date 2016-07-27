# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sc_filters_test))

$(eval $(call gb_CppunitTest_add_exception_objects,sc_filters_test, \
    sc/qa/unit/filters-test \
))

ifeq ($(DISABLE_CVE_TESTS),TRUE)
$(eval $(call gb_CppunitTest_add_defs,sc_filters_test,\
    -DDISABLE_CVE_TESTS \
))
endif

$(eval $(call gb_CppunitTest_use_externals,sc_filters_test, \
	boost_headers \
	mdds_headers \
))

$(eval $(call gb_CppunitTest_use_libraries,sc_filters_test, \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    drawinglayer \
    editeng \
    for \
    forui \
    i18nlangtag \
    msfilter \
    oox \
    sal \
    salhelper \
    sax \
    sc \
    scqahelper \
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
    vcl \
    xo \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_set_include,sc_filters_test,\
    -I$(SRCDIR)/sc/source/ui/inc \
    -I$(SRCDIR)/sc/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sc_filters_test))

$(eval $(call gb_CppunitTest_use_ure,sc_filters_test))
$(eval $(call gb_CppunitTest_use_vcl,sc_filters_test))

$(eval $(call gb_CppunitTest_use_components,sc_filters_test,\
    basic/util/sb \
    chart2/source/chartcore \
    chart2/source/controller/chartcontroller \
    comphelper/util/comphelp \
    configmgr/source/configmgr \
    connectivity/source/manager/sdbc2 \
    dbaccess/util/dba \
    embeddedobj/util/embobj \
    eventattacher/source/evtatt \
    filter/source/config/cache/filterconfig1 \
    forms/util/frm \
    framework/util/fwk \
    i18npool/util/i18npool \
    linguistic/source/lng \
    oox/util/oox \
    package/source/xstor/xstor \
    package/util/package2 \
    sax/source/expatwrap/expwrap \
    scaddins/source/analysis/analysis \
    scaddins/source/datefunc/date \
    sc/util/sc \
    sc/util/scfilt \
    sfx2/util/sfx \
    sot/util/sot \
    svl/util/svl \
    svtools/util/svt \
    svx/util/svx \
    svx/util/svxcore \
    toolkit/util/tk \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    ucb/source/ucp/tdoc/ucptdoc1 \
    unotools/util/utl \
    unoxml/source/rdf/unordf \
    unoxml/source/service/unoxml \
    uui/util/uui \
    xmloff/util/xo \
))

$(eval $(call gb_CppunitTest_use_configuration,sc_filters_test))

# vim: set noet sw=4 ts=4:
