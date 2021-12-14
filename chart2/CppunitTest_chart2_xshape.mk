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

$(eval $(call gb_CppunitTest_CppunitTest,chart2_xshape))

$(eval $(call gb_CppunitTest_use_externals,chart2_xshape, \
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_add_exception_objects,chart2_xshape, \
    chart2/qa/extras/xshape/chart2xshape \
))

$(eval $(call gb_CppunitTest_use_libraries,chart2_xshape, \
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
    vcl \
    oox \
    sal \
    salhelper \
    sax \
    sb \
    sc \
    sw \
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
    $(call gb_Helper_optional,SCRIPTING, \
        vbahelper) \
    xo \
    sw \
))

$(eval $(call gb_CppunitTest_set_include,chart2_xshape,\
	-I$(SRCDIR)/chart2/qa/extras \
    -I$(SRCDIR)/chart2/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,chart2_xshape))

$(eval $(call gb_CppunitTest_use_ure,chart2_xshape))
$(eval $(call gb_CppunitTest_use_vcl,chart2_xshape))

$(eval $(call gb_CppunitTest_use_components,chart2_xshape,\
    basic/util/sb \
    animations/source/animcore/animcore \
    chart2/source/controller/chartcontroller \
    chart2/source/chartcore \
    comphelper/util/comphelp \
    configmgr/source/configmgr \
    embeddedobj/util/embobj \
    eventattacher/source/evtatt \
    filter/source/config/cache/filterconfig1 \
    filter/source/storagefilterdetect/storagefd \
    forms/util/frm \
    framework/util/fwk \
    i18npool/util/i18npool \
    linguistic/source/lng \
    oox/util/oox \
    package/source/xstor/xstor \
    package/util/package2 \
    sax/source/expatwrap/expwrap \
    sc/util/sc \
    sc/util/scd \
    sc/util/scfilt \
    sw/util/sw \
    sw/util/swd \
    sw/util/msword \
    sd/util/sd \
    sd/util/sdfilt \
    sd/util/sdd \
    $(call gb_Helper_optional,SCRIPTING, \
	    sc/util/vbaobj) \
    scaddins/source/analysis/analysis \
    scaddins/source/datefunc/date \
    scripting/source/basprov/basprov \
    scripting/util/scriptframe \
    sfx2/util/sfx \
    sot/util/sot \
    svl/source/fsstor/fsstorage \
    svl/util/svl \
    svtools/util/svt \
    svx/util/svx \
    svx/util/svxcore \
    toolkit/util/tk \
    vcl/vcl.common \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    ucb/source/ucp/tdoc/ucptdoc1 \
    uui/util/uui \
    unotools/util/utl \
    unoxml/source/rdf/unordf \
    unoxml/source/service/unoxml \
    writerfilter/util/writerfilter \
    xmloff/util/xo \
))

$(eval $(call gb_CppunitTest_use_configuration,chart2_xshape))

$(eval $(call gb_CppunitTest_use_uiconfigs,chart2_xshape, \
    modules/scalc \
    modules/simpress \
    sfx \
    svt \
))

# vim: set noet sw=4 ts=4:
