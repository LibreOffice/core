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

$(eval $(call gb_CppunitTest_CppunitTest,desktop_lib))

$(eval $(call gb_CppunitTest_add_exception_objects,desktop_lib, \
	desktop/qa/desktop_lib/test_desktop_lib \
))

$(eval $(call gb_CppunitTest_use_libraries,desktop_lib, \
	comphelper \
	cppu \
	cppuhelper \
	sal \
	sfx \
	sofficeapp \
	subsequenttest \
	sw \
	test \
	unotest \
	utl \
	vcl \
    $(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_externals,desktop_lib, \
    boost_headers \
    cairo \
))

$(eval $(call gb_CppunitTest_set_include,desktop_lib,\
    -I$(SRCDIR)/desktop/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,desktop_lib))

$(eval $(call gb_CppunitTest_use_ure,desktop_lib))

$(eval $(call gb_CppunitTest_use_vcl,desktop_lib))

$(eval $(call gb_CppunitTest_use_components,desktop_lib,\
    comphelper/util/comphelp \
    configmgr/source/configmgr \
    dtrans/util/mcnttype \
    filter/source/config/cache/filterconfig1 \
    filter/source/storagefilterdetect/storagefd \
    framework/util/fwk \
    i18npool/util/i18npool \
    package/source/xstor/xstor \
    package/util/package2 \
    sax/source/expatwrap/expwrap \
    sfx2/util/sfx \
    svl/source/fsstor/fsstorage \
    svtools/util/svt \
    sw/util/sw \
    sw/util/swd \
    sc/util/sc \
    sc/util/scd \
    sd/util/sd \
    sd/util/sdd \
    toolkit/util/tk \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    unoxml/source/service/unoxml \
    uui/util/uui \
    xmloff/util/xo \
    i18npool/source/search/i18nsearch \
    filter/source/graphic/graphicfilter \
    linguistic/source/lng \
))

$(eval $(call gb_CppunitTest_use_configuration,desktop_lib))

# vim: set noet sw=4 ts=4:
