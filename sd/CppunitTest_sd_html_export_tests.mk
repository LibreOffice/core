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

$(eval $(call gb_CppunitTest_CppunitTest,sd_html_export_tests))

$(eval $(call gb_CppunitTest_use_externals,sd_html_export_tests,\
	boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_add_exception_objects,sd_html_export_tests, \
    sd/qa/unit/HtmlExportTest \
))

$(eval $(call gb_CppunitTest_use_libraries,sd_html_export_tests, \
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
    sd \
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

$(eval $(call gb_CppunitTest_set_include,sd_html_export_tests,\
    -I$(SRCDIR)/sd/source/ui/inc \
    -I$(SRCDIR)/sd/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sd_html_export_tests))

$(eval $(call gb_CppunitTest_use_ure,sd_html_export_tests))
$(eval $(call gb_CppunitTest_use_vcl,sd_html_export_tests))

$(eval $(call gb_CppunitTest_use_components,sd_html_export_tests,\
    animations/source/animcore/animcore \
    basic/util/sb \
    chart2/source/chartcore \
    chart2/source/controller/chartcontroller \
    comphelper/util/comphelp \
    configmgr/source/configmgr \
    dbaccess/util/dba \
    desktop/source/deployment/deployment \
    embeddedobj/util/embobj \
    filter/source/config/cache/filterconfig1 \
    filter/source/svg/svgfilter \
    forms/util/frm \
    framework/util/fwk \
    i18npool/util/i18npool \
    linguistic/source/lng \
    oox/util/oox \
    package/source/xstor/xstor \
    package/util/package2 \
    sax/source/expatwrap/expwrap \
    sd/util/sd \
    sd/util/sdfilt \
    sd/util/sdd \
    sfx2/util/sfx \
    sot/util/sot \
    svl/source/fsstor/fsstorage \
    svtools/util/svt \
    svx/util/svxcore \
    toolkit/util/tk \
    ucb/source/core/ucb1 \
    ucb/source/ucp/expand/ucpexpand1 \
    ucb/source/ucp/file/ucpfile1 \
    ucb/source/ucp/package/ucppkg1 \
    ucb/source/ucp/tdoc/ucptdoc1 \
    unotools/util/utl \
    unoxml/source/rdf/unordf \
    unoxml/source/service/unoxml \
    uui/util/uui \
    xmloff/util/xo \
    xmlsecurity/util/xsec_fw \
    xmlsecurity/util/xmlsecurity \
))

$(eval $(call gb_CppunitTest_use_configuration,sd_html_export_tests))

$(call gb_CppunitTest_get_target,sd_html_export_tests) : $(call gb_AllLangResTarget_get_target,sd)

# vim: set noet sw=4 ts=4:
