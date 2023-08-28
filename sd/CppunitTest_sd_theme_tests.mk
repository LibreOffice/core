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

$(eval $(call gb_CppunitTest_CppunitTest,sd_theme_tests))

$(eval $(call gb_CppunitTest_use_externals,sd_theme_tests,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sd_theme_tests))

$(eval $(call gb_CppunitTest_add_exception_objects,sd_theme_tests, \
    sd/qa/unit/ThemeTest \
))

$(eval $(call gb_CppunitTest_use_libraries,sd_theme_tests, \
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
    sal \
    salhelper \
    sax \
    sd \
    sfx \
    sot \
    subsequenttest \
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

$(eval $(call gb_CppunitTest_set_include,sd_theme_tests,\
    -I$(SRCDIR)/sd/source/ui/inc \
    -I$(SRCDIR)/sd/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sd_theme_tests))
$(eval $(call gb_CppunitTest_use_ure,sd_theme_tests))
$(eval $(call gb_CppunitTest_use_vcl,sd_theme_tests))
$(eval $(call gb_CppunitTest_use_rdb,sd_theme_tests,services))

$(eval $(call gb_CppunitTest_use_custom_headers,sd_theme_tests,\
    officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,sd_theme_tests))

$(eval $(call gb_CppunitTest_add_arguments,sd_theme_tests, \
    -env:arg-env=$(gb_Helper_LIBRARY_PATH_VAR)"$$$${$(gb_Helper_LIBRARY_PATH_VAR)+=$$$$$(gb_Helper_LIBRARY_PATH_VAR)}" \
))

# vim: set noet sw=4 ts=4:
