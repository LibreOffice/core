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

$(eval $(call gb_CppunitTest_CppunitTest,sw_layout_test))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_layout_test, \
    sw/qa/core/layout-test \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_layout_test, \
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
    sw \
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
    vcl \
    xo \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_set_include,sw_layout_test,\
    -I$(SRCDIR)/sw/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_layout_test,\
    offapi \
    oovbaapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_layout_test))

$(eval $(call gb_CppunitTest_use_rdb,sw_layout_test,services))

$(eval $(call gb_CppunitTest_use_configuration,sw_layout_test))

# vim: set noet sw=4 ts=4:
