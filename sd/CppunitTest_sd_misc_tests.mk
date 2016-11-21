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

$(eval $(call gb_CppunitTest_CppunitTest,sd_misc_tests))

$(eval $(call gb_CppunitTest_use_externals,sd_misc_tests,\
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_add_exception_objects,sd_misc_tests, \
    sd/qa/unit/misc-tests \
))

$(eval $(call gb_CppunitTest_use_libraries,sd_misc_tests, \
    $(call gb_Helper_optional,AVMEDIA,avmedia) \
    basegfx \
    canvastools \
    comphelper \
    cppcanvas \
    cppu \
    cppuhelper \
    drawinglayer \
    editeng \
    for \
    forui \
    i18nlangtag \
    i18nutil \
    msfilter \
    oox \
    sal \
    salhelper \
    sax \
    sb \
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
    $(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_set_include,sd_misc_tests,\
    -I$(SRCDIR)/sd/inc \
    -I$(SRCDIR)/sd/source/ui/inc \
    -I$(SRCDIR)/sd/source/ui/slidesorter/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sd_misc_tests))

$(eval $(call gb_CppunitTest_use_externals,sd_misc_tests,\
    boost_headers \
))

$(eval $(call gb_CppunitTest_use_ure,sd_misc_tests))
$(eval $(call gb_CppunitTest_use_vcl,sd_misc_tests))

$(eval $(call gb_CppunitTest_use_rdb,sd_misc_tests,services))

$(eval $(call gb_CppunitTest_use_configuration,sd_misc_tests))

$(call gb_CppunitTest_get_target,sd_misc_tests) : $(call gb_AllLangResTarget_get_target,sd)

# vim: set noet sw=4 ts=4:
