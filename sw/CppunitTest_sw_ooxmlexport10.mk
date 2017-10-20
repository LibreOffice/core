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

$(eval $(call gb_CppunitTest_CppunitTest,sw_ooxmlexport10))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_ooxmlexport10, \
    sw/qa/extras/ooxmlexport/ooxmlexport10 \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_ooxmlexport10, \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    test \
    unotest \
    utl \
    sfx \
    sw \
    tl \
    vcl \
    svxcore \
))

$(eval $(call gb_CppunitTest_use_externals,sw_ooxmlexport10,\
	boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_ooxmlexport10,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/qa/extras/inc \
    $$(INCLUDE) \
))

ifeq ($(OS),MACOSX)

$(eval $(call gb_CppunitTest_add_cxxflags,sw_ooxmlexport10,\
    $(gb_OBJCXXFLAGS) \
))

$(eval $(call gb_CppunitTest_use_system_darwin_frameworks,sw_ooxmlexport10,\
	AppKit \
))

endif

$(eval $(call gb_CppunitTest_use_sdk_api,sw_ooxmlexport10))

$(eval $(call gb_CppunitTest_use_ure,sw_ooxmlexport10))
$(eval $(call gb_CppunitTest_use_vcl,sw_ooxmlexport10))

$(eval $(call gb_CppunitTest_use_components,sw_ooxmlexport10,\
	basic/util/sb \
    chart2/source/controller/chartcontroller \
    chart2/source/chartcore \
	canvas/source/factory/canvasfactory \
    comphelper/util/comphelp \
    configmgr/source/configmgr \
    drawinglayer/drawinglayer \
    embeddedobj/util/embobj \
    emfio/emfio \
    filter/source/config/cache/filterconfig1 \
    forms/util/frm \
    framework/util/fwk \
    i18npool/util/i18npool \
    linguistic/source/lng \
    oox/util/oox \
    package/source/xstor/xstor \
    package/util/package2 \
    sax/source/expatwrap/expwrap \
    sw/util/sw \
    sw/util/swd \
    sw/util/msword \
    sfx2/util/sfx \
	starmath/util/sm \
    svl/source/fsstor/fsstorage \
    svl/util/svl \
    svtools/util/svt \
    svx/util/svx \
    svx/util/svxcore \
    toolkit/util/tk \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    unotools/util/utl \
    unoxml/source/service/unoxml \
    unoxml/source/rdf/unordf \
    uui/util/uui \
    writerfilter/util/writerfilter \
    xmloff/util/xo \
))

$(eval $(call gb_CppunitTest_use_configuration,sw_ooxmlexport10))

$(eval $(call gb_CppunitTest_use_uiconfigs,sw_ooxmlexport10,\
    modules/swriter \
))

# vim: set noet sw=4 ts=4:
