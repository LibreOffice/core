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

$(eval $(call gb_CppunitTest_CppunitTest,sd_filters_test))

$(eval $(call gb_CppunitTest_use_external,sd_filters_test,boost_headers))

$(eval $(call gb_CppunitTest_add_exception_objects,sd_filters_test, \
    sd/qa/unit/filters-test \
))

ifeq ($(DISABLE_CVE_TESTS),TRUE)
$(eval $(call gb_CppunitTest_add_defs,sd_filters_test,\
    -DDISABLE_CVE_TESTS \
))
endif

$(eval $(call gb_CppunitTest_use_libraries,sd_filters_test, \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    drawinglayer \
    editeng \
    i18nlangtag \
    i18nutil \
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

$(eval $(call gb_CppunitTest_set_include,sd_filters_test,\
    -I$(SRCDIR)/sd/source/ui/inc \
    -I$(SRCDIR)/sd/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sd_filters_test))

$(eval $(call gb_CppunitTest_use_ure,sd_filters_test))
$(eval $(call gb_CppunitTest_use_vcl,sd_filters_test))

$(eval $(call gb_CppunitTest_use_rdb,sd_filters_test,services))

$(eval $(call gb_CppunitTest_use_configuration,sd_filters_test))

# sd dlopens libicg.so for cgm import, so ensure its built by now
$(call gb_CppunitTest_get_target,sd_filters_test) : | \
    $(call gb_Library_get_target,icg) \

# vim: set noet sw=4 ts=4:
