# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sc_rangelst_test))

$(eval $(call gb_CppunitTest_use_externals,sc_rangelst_test, \
	boost_headers \
    icu_headers \
    icui18n \
    icuuc \
	libxml2 \
	mdds_headers \
))

$(eval $(call gb_CppunitTest_add_exception_objects,sc_rangelst_test, \
    sc/qa/unit/rangelst_test \
))

$(eval $(call gb_CppunitTest_use_libraries,sc_rangelst_test, \
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
    oox \
    sal \
    salhelper \
    sax \
    sb \
    sc \
    scqahelper \
    sfx \
    sot \
    svl \
    svt \
    svx \
    svxcore \
	test \
    tk \
    tl \
    ucbhelper \
	unotest \
    utl \
    vbahelper \
    vcl \
    xo \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_set_include,sc_rangelst_test,\
    -I$(SRCDIR)/sc/source/ui/inc \
    -I$(SRCDIR)/sc/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sc_rangelst_test,\
    offapi \
    oovbaapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,sc_rangelst_test))
$(eval $(call gb_CppunitTest_use_vcl,sc_rangelst_test))

$(eval $(call gb_CppunitTest_use_rdb,sc_rangelst_test,services))

$(eval $(call gb_CppunitTest_use_configuration,sc_rangelst_test))

# vim: set noet sw=4 ts=4:
