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

# template for unoapi tests
define sw_unoapi_common

$(eval $(call gb_CppunitTest_CppunitTest,sw_apitests_$(1)))

$(eval $(call gb_CppunitTest_use_externals,sw_apitests_$(1), \
	boost_headers \
	libxml2 \
))


$(eval $(call gb_CppunitTest_use_common_precompiled_header,sw_apitests_$(1)))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_apitests_$(1), \
    sw/qa/api/$(1) \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_apitests_$(1), \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    drawinglayer \
    docmodel \
    editeng \
    for \
    forui \
    i18nlangtag \
    msfilter \
    oox \
    sal \
    salhelper \
    sax \
    sb \
    sw \
    sfx \
    sot \
    svl \
    svt \
    svx \
    svxcore \
    subsequenttest \
    test \
    tl \
    tk \
    ucbhelper \
    unotest \
    utl \
    $(call gb_Helper_optional,SCRIPTING, \
        vbahelper) \
    vcl \
    xo \
))

$(eval $(call gb_CppunitTest_set_include,sw_apitests_$(1),\
    -I$(SRCDIR)/sw/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_apitests_$(1),\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_apitests_$(1)))
$(eval $(call gb_CppunitTest_use_vcl,sw_apitests_$(1)))
$(eval $(call gb_CppunitTest_use_configuration,sw_apitests_$(1)))

$(eval $(call gb_CppunitTest_use_rdb,sw_apitests_$(1),services))

$(eval $(call gb_CppunitTest_use_uiconfigs,sw_apitests_$(1), \
    svt \
    svx \
))

$(call gb_CppunitTest_get_target,sw_apitests_$(1)): $(call gb_AllLangPackage_get_target,autotextshare)
endef

# vim: set noet sw=4 ts=4:
