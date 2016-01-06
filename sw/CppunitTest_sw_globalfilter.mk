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

$(eval $(call gb_CppunitTest_CppunitTest,sw_globalfilter))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_globalfilter, \
    sw/qa/extras/globalfilter/globalfilter \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_globalfilter, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    svt \
    sw \
    test \
    tl \
    unotest \
    utl \
    vcl \
    $(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_externals,sw_globalfilter,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_globalfilter,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/qa/extras/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_globalfilter,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_globalfilter))
$(eval $(call gb_CppunitTest_use_vcl,sw_globalfilter))

$(eval $(call gb_CppunitTest_use_components,sw_globalfilter,\
	basic/util/sb \
	comphelper/util/comphelp \
	configmgr/source/configmgr \
	dbaccess/util/dba \
	drawinglayer/drawinglayer \
	embeddedobj/util/embobj \
	filter/source/config/cache/filterconfig1 \
	filter/source/storagefilterdetect/storagefd \
	filter/source/textfilterdetect/textfd \
	forms/util/frm \
	framework/util/fwk \
	i18npool/util/i18npool \
	linguistic/source/lng \
	oox/util/oox \
	package/source/xstor/xstor \
	package/util/package2 \
	sax/source/expatwrap/expwrap \
	sfx2/util/sfx \
	svl/source/fsstor/fsstorage \
	svtools/util/svt \
	sw/util/msword \
	sw/util/sw \
	sw/util/swd \
	toolkit/util/tk \
	ucb/source/core/ucb1 \
	ucb/source/ucp/file/ucpfile1 \
	unotools/util/utl \
	unoxml/source/rdf/unordf \
	unoxml/source/service/unoxml \
	uui/util/uui \
	$(if $(filter DESKTOP,$(BUILD_TYPE)),xmlhelp/util/ucpchelp1) \
	writerfilter/util/writerfilter \
	xmloff/util/xo \
))

$(eval $(call gb_CppunitTest_use_custom_headers,sw_globalfilter,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,sw_globalfilter))

# vim: set noet sw=4 ts=4:
