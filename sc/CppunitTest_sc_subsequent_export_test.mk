# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sc_subsequent_export_test))

$(eval $(call gb_CppunitTest_add_exception_objects,sc_subsequent_export_test, \
    sc/qa/unit/subsequent_export-test \
))

$(eval $(call gb_CppunitTest_use_externals,sc_subsequent_export_test, \
	boost_headers \
	mdds_headers \
	orcus \
	orcus-parser \
))

$(eval $(call gb_CppunitTest_use_libraries,sc_subsequent_export_test, \
    avmedia \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    drawinglayer \
    editeng \
    fileacc \
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
    tl \
    tk \
    ucbhelper \
	unotest \
    utl \
    vcl \
    xo \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_set_include,sc_subsequent_export_test,\
    -I$(SRCDIR)/sc/source/ui/inc \
    -I$(SRCDIR)/sc/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sc_subsequent_export_test,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,sc_subsequent_export_test))

$(eval $(call gb_CppunitTest_use_components,sc_subsequent_export_test,\
    chart2/source/controller/chartcontroller \
    chart2/source/chartcore \
    comphelper/util/comphelp \
    configmgr/source/configmgr \
    dbaccess/util/dba \
    embeddedobj/util/embobj \
    eventattacher/source/evtatt \
    fileaccess/source/fileacc \
    filter/source/config/cache/filterconfig1 \
    forms/util/frm \
    framework/util/fwk \
    i18npool/util/i18npool \
    i18npool/source/search/i18nsearch \
    linguistic/source/lng \
    oox/util/oox \
    package/source/xstor/xstor \
    package/util/package2 \
    sax/source/expatwrap/expwrap \
    sax/source/fastparser/fastsax \
    sc/util/sc \
    sc/util/scfilt \
    scaddins/source/analysis/analysis \
    scaddins/source/datefunc/date \
    sfx2/util/sfx \
    sot/util/sot \
    svl/util/svl \
    svtools/util/svt \
    toolkit/util/tk \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    ucb/source/ucp/tdoc/ucptdoc1 \
    unotools/util/utl \
    unoxml/source/rdf/unordf \
    unoxml/source/service/unoxml \
    xmloff/util/xo \
    xmlsecurity/util/xsec_fw \
    xmlsecurity/util/xmlsecurity \
))

ifeq ($(OS),WNT)
$(eval $(call gb_CppunitTest_use_components,sc_subsequent_export_test,\
    xmlsecurity/util/xsec_xmlsec.windows \
))
else
$(eval $(call gb_CppunitTest_use_components,sc_subsequent_export_test,\
    xmlsecurity/util/xsec_xmlsec \
))
endif

$(eval $(call gb_CppunitTest_use_configuration,sc_subsequent_export_test))

$(eval $(call gb_CppunitTest_use_filter_configuration,sc_subsequent_export_test))

# vim: set noet sw=4 ts=4:
