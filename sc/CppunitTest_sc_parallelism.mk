# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sc_parallelism))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sc_parallelism))

$(eval $(call gb_CppunitTest_add_exception_objects,sc_parallelism, \
    sc/qa/unit/parallelism \
))

$(eval $(call gb_CppunitTest_use_library_objects,sc_parallelism, \
    sc \
    scqahelper \
))

$(eval $(call gb_CppunitTest_use_externals,sc_parallelism, \
	boost_headers \
    $(call gb_Helper_optional,OPENCL,clew) \
    icu_headers \
    icui18n \
    icuuc \
    libxml2 \
    mdds_headers \
    orcus \
    orcus-parser \
))

$(eval $(call gb_CppunitTest_use_libraries,sc_parallelism, \
    $(call gb_Helper_optional,AVMEDIA,avmedia) \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    dbtools \
    drawinglayer \
    drawinglayercore \
    editeng \
    for \
    forui \
    i18nlangtag \
    i18nutil \
    $(call gb_Helper_optional,OPENCL,opencl) \
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
))

$(eval $(call gb_CppunitTest_set_include,sc_parallelism,\
    -I$(SRCDIR)/sc/source/ui/inc \
    -I$(SRCDIR)/sc/source/core/inc \
    -I$(SRCDIR)/sc/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sc_parallelism,\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_CppunitTest_use_custom_headers,sc_parallelism,\
        officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_ure,sc_parallelism))
$(eval $(call gb_CppunitTest_use_vcl,sc_parallelism))

$(eval $(call gb_CppunitTest_use_components,sc_parallelism,\
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
    vcl/vcl.common \
))

$(eval $(call gb_CppunitTest_use_configuration,sc_parallelism))

# vim: set noet sw=4 ts=4:
