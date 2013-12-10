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

$(eval $(call gb_CppunitTest_CppunitTest,sw_ooxmlexport))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_ooxmlexport, \
    sw/qa/extras/ooxmlexport/ooxmlexport \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_ooxmlexport, \
    comphelper \
    cppu \
	cppuhelper \
    sal \
    sw \
    test \
	tl \
    unotest \
    utl \
    vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_externals,sw_ooxmlexport,\
	boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_ooxmlexport,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
	-I$(SRCDIR)/sw/qa/extras/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_ooxmlexport,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_ooxmlexport))

$(eval $(call gb_CppunitTest_use_components,sw_ooxmlexport,\
	basic/util/sb \
	canvas/source/factory/canvasfactory \
    comphelper/util/comphelp \
    configmgr/source/configmgr \
	drawinglayer/drawinglayer \
    embeddedobj/util/embobj \
    filter/source/config/cache/filterconfig1 \
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
    svtools/util/svt \
	svx/util/svx \
    toolkit/util/tk \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    unotools/util/utl \
    unoxml/source/service/unoxml \
	uui/util/uui \
    writerfilter/util/writerfilter \
    xmloff/util/xo \
))

$(eval $(call gb_CppunitTest_use_configuration,sw_ooxmlexport))

$(eval $(call gb_CppunitTest_use_unittest_configuration,sw_ooxmlexport))

# vim: set noet sw=4 ts=4:
