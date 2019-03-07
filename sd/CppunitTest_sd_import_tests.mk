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

$(eval $(call gb_CppunitTest_CppunitTest,sd_import_tests))

$(eval $(call gb_CppunitTest_use_externals,sd_import_tests,\
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_add_exception_objects,sd_import_tests, \
    sd/qa/unit/import-tests \
))

$(eval $(call gb_CppunitTest_use_libraries,sd_import_tests, \
	$(call gb_Helper_optional,AVMEDIA,avmedia) \
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
))

$(eval $(call gb_CppunitTest_set_include,sd_import_tests,\
    -I$(SRCDIR)/sd/source/ui/inc \
    -I$(SRCDIR)/sd/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sd_import_tests))

$(eval $(call gb_CppunitTest_use_ure,sd_import_tests))
$(eval $(call gb_CppunitTest_use_vcl,sd_import_tests))

$(eval $(call gb_CppunitTest_use_components,sd_import_tests,\
    animations/source/animcore/animcore \
    basic/util/sb \
    chart2/source/chartcore \
    chart2/source/controller/chartcontroller \
    comphelper/util/comphelp \
    configmgr/source/configmgr \
    dbaccess/util/dba \
    desktop/source/deployment/deployment \
    drawinglayer/drawinglayer \
    embeddedobj/util/embobj \
    emfio/emfio \
    filter/source/config/cache/filterconfig1 \
	filter/source/odfflatxml/odfflatxml \
    filter/source/svg/svgfilter \
    filter/source/xmlfilteradaptor/xmlfa \
	filter/source/xmlfilterdetect/xmlfd \
	filter/source/storagefilterdetect/storagefd \
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
    $(if $(ENABLE_POPPLER),sdext/source/pdfimport/pdfimport) \
    sfx2/util/sfx \
    sot/util/sot \
    svl/source/fsstor/fsstorage \
    svtools/util/svt \
    svx/util/svxcore \
    svgio/svgio \
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
    vcl/vcl.common \
    xmlscript/util/xmlscript \
    xmloff/util/xo \
    xmlsecurity/util/xmlsecurity \
))

$(eval $(call gb_CppunitTest_use_configuration,sd_import_tests))

$(eval $(call gb_CppunitTest_use_packages,sd_import_tests,\
	oox_customshapes \
))

ifneq ($(ENABLE_POPPLER),)
$(eval $(call gb_CppunitTest_use_executable,sd_import_tests,xpdfimport))
endif

# vim: set noet sw=4 ts=4:
