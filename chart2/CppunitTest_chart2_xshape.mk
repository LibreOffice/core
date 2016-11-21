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

$(eval $(call gb_CppunitTest_CppunitTest,chart2_xshape))

$(eval $(call gb_CppunitTest_use_externals,chart2_xshape, \
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_add_exception_objects,chart2_xshape, \
    chart2/qa/extras/xshape/chart2xshape \
))

$(eval $(call gb_CppunitTest_use_libraries,chart2_xshape, \
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

$(eval $(call gb_CppunitTest_set_include,chart2_xshape,\
	-I$(SRCDIR)/chart2/qa/extras \
    -I$(SRCDIR)/chart2/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,chart2_xshape))

$(eval $(call gb_CppunitTest_use_ure,chart2_xshape))
$(eval $(call gb_CppunitTest_use_vcl,chart2_xshape))

$(eval $(call gb_CppunitTest_use_rdb,chart2_xshape,services))

$(eval $(call gb_CppunitTest_use_configuration,chart2_xshape))

# vim: set noet sw=4 ts=4:
