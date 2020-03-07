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

$(eval $(call gb_CppunitTest_CppunitTest,sw_uwriter))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sw_uwriter))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_uwriter, \
    sw/qa/core/uwriter \
    sw/qa/core/Test-BigPtrArray \
    sw/qa/core/test_ToxWhitespaceStripper \
    sw/qa/core/test_ToxLinkProcessor \
    sw/qa/core/test_ToxTextGenerator \
    sw/qa/core/test_ToxMiscTest \
))

$(eval $(call gb_CppunitTest_use_library_objects,sw_uwriter,sw))

$(eval $(call gb_CppunitTest_use_libraries,sw_uwriter, \
	$(call gb_Helper_optional,AVMEDIA,avmedia) \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    $(call gb_Helper_optional,DBCONNECTIVITY, \
        dbtools) \
    drawinglayer \
    drawinglayercore \
    editeng \
    i18nlangtag \
    i18nutil \
    lng \
    msfilter \
    oox \
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
    swd \
	test \
    tk \
    tl \
    ucbhelper \
    unotest \
    utl \
    vbahelper \
    vcl \
	xmlreader \
    xo \
))

$(eval $(call gb_CppunitTest_use_externals,sw_uwriter,\
	boost_headers \
	icui18n \
	icuuc \
	icu_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_uwriter,\
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_uwriter,\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_uwriter))
$(eval $(call gb_CppunitTest_use_vcl,sw_uwriter))

$(eval $(call gb_CppunitTest_use_components,sw_uwriter,\
    comphelper/util/comphelp \
    configmgr/source/configmgr \
    framework/util/fwk \
    i18npool/util/i18npool \
    package/util/package2 \
    package/source/xstor/xstor \
    sfx2/util/sfx \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    unotools/util/utl \
    unoxml/source/service/unoxml \
    uui/util/uui \
))

$(eval $(call gb_CppunitTest_use_configuration,sw_uwriter))

# vim: set noet sw=4 ts=4:
