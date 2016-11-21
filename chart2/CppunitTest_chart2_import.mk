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

$(eval $(call gb_CppunitTest_CppunitTest,chart2_import))

$(eval $(call gb_CppunitTest_use_externals,chart2_import, \
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_add_exception_objects,chart2_import, \
    chart2/qa/extras/chart2import \
))

$(eval $(call gb_CppunitTest_use_libraries,chart2_import, \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    drawinglayer \
    editeng \
    for \
    forui \
    i18nlangtag \
    msfilter \
    vcl \
    oox \
    sal \
    salhelper \
    sax \
    sb \
    sc \
    sw \
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
    vbahelper \
    xo \
    sw \
    $(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_set_include,chart2_import,\
    -I$(SRCDIR)/chart2/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,chart2_import))

$(eval $(call gb_CppunitTest_use_ure,chart2_import))
$(eval $(call gb_CppunitTest_use_vcl,chart2_import))

$(eval $(call gb_CppunitTest_use_rdb,chart2_import,services))

$(eval $(call gb_CppunitTest_use_configuration,chart2_import))

$(call gb_CppunitTest_get_target,chart2_import): \
    $(call gb_AllLangResTarget_get_target,sd)

# vim: set noet sw=4 ts=4:
