# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sc_dataprovider))

$(eval $(call gb_CppunitTest_add_exception_objects,sc_dataprovider, \
    sc/qa/unit/dataproviders_test \
))

$(eval $(call gb_CppunitTest_use_externals,sc_dataprovider, \
	boost_headers \
	mdds_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_use_libraries,sc_dataprovider, \
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
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_set_include,sc_dataprovider,\
    -I$(SRCDIR)/sc/source/ui/inc \
    -I$(SRCDIR)/sc/inc \
	-I$(SRCDIR)/sc/source/filter/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_custom_headers,sc_dataprovider,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_api,sc_dataprovider,\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sc_dataprovider))
$(eval $(call gb_CppunitTest_use_vcl,sc_dataprovider))

$(eval $(call gb_CppunitTest_use_components,sc_dataprovider,\
	basic/util/sb \
    chart2/source/chartcore \
    chart2/source/controller/chartcontroller \
    comphelper/util/comphelp \
    configmgr/source/configmgr \
    connectivity/source/cpool/dbpool2 \
    connectivity/source/drivers/hsqldb/hsqldb \
    $(if $(ENABLE_FIREBIRD_SDBC),connectivity/source/drivers/firebird/firebird_sdbc) \
    connectivity/source/drivers/jdbc/jdbc \
    connectivity/source/manager/sdbc2 \
    dbaccess/source/filter/xml/dbaxml \
    dbaccess/util/dba \
    dbaccess/util/dbu \
    embeddedobj/util/embobj \
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
    vcl/vcl.common \
    xmloff/util/xo \
    xmlsecurity/util/xmlsecurity \
))

$(eval $(call gb_CppunitTest_use_externals,sc_dataprovider,\
	orcus \
	orcus-parser \
	boost_filesystem \
	boost_system \
	boost_iostreams \
	zlib \
))

$(eval $(call gb_CppunitTest_use_configuration,sc_dataprovider))

# vim: set noet sw=4 ts=4:
