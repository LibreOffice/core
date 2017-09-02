# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sc_subsequent_filters_test))

$(eval $(call gb_CppunitTest_add_exception_objects,sc_subsequent_filters_test, \
    sc/qa/unit/subsequent_filters-test \
))

$(eval $(call gb_CppunitTest_use_externals,sc_subsequent_filters_test, \
	boost_headers \
	mdds_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_use_libraries,sc_subsequent_filters_test, \
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
    sb \
    sc \
    scqahelper \
    sfx \
    sot \
    subsequenttest \
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
))

$(eval $(call gb_CppunitTest_set_include,sc_subsequent_filters_test,\
    -I$(SRCDIR)/sc/source/ui/inc \
    -I$(SRCDIR)/sc/inc \
	-I$(SRCDIR)/sc/source/filter/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sc_subsequent_filters_test))

$(eval $(call gb_CppunitTest_use_ure,sc_subsequent_filters_test))
$(eval $(call gb_CppunitTest_use_vcl,sc_subsequent_filters_test))

$(eval $(call gb_CppunitTest_use_components,sc_subsequent_filters_test,\
	basic/util/sb \
    chart2/source/chartcore \
    chart2/source/controller/chartcontroller \
    comphelper/util/comphelp \
    configmgr/source/configmgr \
    dbaccess/util/dba \
    embeddedobj/util/embobj \
    emfio/emfio \
    eventattacher/source/evtatt \
    filter/source/config/cache/filterconfig1 \
    forms/util/frm \
    framework/util/fwk \
    i18npool/source/search/i18nsearch \
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
    svl/source/fsstor/fsstorage \
    svtools/util/svt \
    toolkit/util/tk \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    ucb/source/ucp/tdoc/ucptdoc1 \
    unotools/util/utl \
    unoxml/source/rdf/unordf \
    unoxml/source/service/unoxml \
    uui/util/uui \
    xmloff/util/xo \
    xmlsecurity/util/xmlsecurity \
))

ifeq ($(OS),WNT)
$(eval $(call gb_CppunitTest_use_components,sc_subsequent_filters_test,\
    xmlsecurity/util/xsec_xmlsec.windows \
))
else
$(eval $(call gb_CppunitTest_use_components,sc_subsequent_filters_test,\
    xmlsecurity/util/xsec_xmlsec \
))
endif

$(eval $(call gb_CppunitTest_use_externals,sc_subsequent_filters_test,\
	orcus \
	orcus-parser \
	boost_filesystem \
	boost_system \
	boost_iostreams \
	zlib \
))

$(eval $(call gb_CppunitTest_use_configuration,sc_subsequent_filters_test))

# vim: set noet sw=4 ts=4:
