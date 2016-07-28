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

$(eval $(call gb_CppunitTest_CppunitTest,sw_filters_test))

$(eval $(call gb_CppunitTest_use_externals,sw_filters_test,\
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_filters_test, \
    sw/qa/core/filters-test \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_filters_test, \
    sw \
    sfx \
    svl \
    svt \
    sot \
	test \
    tl \
    ucbhelper \
    unotest \
    utl \
    vcl \
    i18nlangtag \
    comphelper \
    cppu \
    cppuhelper \
    sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_set_include,sw_filters_test,\
    -I$(SRCDIR)/sw/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sw_filters_test))

$(eval $(call gb_CppunitTest_use_ure,sw_filters_test))
$(eval $(call gb_CppunitTest_use_vcl,sw_filters_test))

$(eval $(call gb_CppunitTest_use_components,sw_filters_test,\
	basic/util/sb \
    comphelper/util/comphelp \
    configmgr/source/configmgr \
    dbaccess/util/dba \
    embeddedobj/util/embobj \
    filter/source/config/cache/filterconfig1 \
    filter/source/t602/t602filter \
    forms/util/frm \
    framework/util/fwk \
    i18npool/util/i18npool \
	linguistic/source/lng \
    oox/util/oox \
    package/source/xstor/xstor \
    package/util/package2 \
    sax/source/expatwrap/expwrap \
    sfx2/util/sfx \
	starmath/util/sm \
    svtools/util/svt \
    sw/util/msword \
    sw/util/sw \
	sw/util/swd \
	uui/util/uui \
    toolkit/util/tk \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    unotools/util/utl \
    unoxml/source/rdf/unordf \
    unoxml/source/service/unoxml \
    writerfilter/util/writerfilter \
	$(if $(filter DESKTOP,$(BUILD_TYPE)),xmlhelp/util/ucpchelp1) \
	xmloff/source/transform/xof \
    xmloff/util/xo \
))

$(eval $(call gb_CppunitTest_use_configuration,sw_filters_test))

# vim: set noet sw=4 ts=4:
