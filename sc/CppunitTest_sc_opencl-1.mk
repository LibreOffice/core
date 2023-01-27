# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sc_opencl-1))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sc_opencl-1))

$(eval $(call gb_CppunitTest_add_exception_objects,sc_opencl-1, \
    sc/qa/unit/opencl-test-1 \
))

$(eval $(call gb_CppunitTest_use_externals,sc_opencl-1, \
	boost_headers \
    $(call gb_Helper_optional,OPENCL,clew) \
	mdds_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_use_libraries,sc_opencl-1, \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    drawinglayer \
    drawinglayercore \
    editeng \
    for \
    forui \
    i18nlangtag \
    msfilter \
    oox \
    sal \
    salhelper \
    sax \
    sc \
    scqahelper \
    sfx \
    sot \
    subsequenttest \
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
    vcl \
    xo \
))

$(eval $(call gb_CppunitTest_set_include,sc_opencl-1,\
    -I$(SRCDIR)/sc/source/ui/inc \
    -I$(SRCDIR)/sc/source/core/inc \
    -I$(SRCDIR)/sc/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sc_opencl-1,\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sc_opencl-1))
$(eval $(call gb_CppunitTest_use_vcl,sc_opencl-1))

$(eval $(call gb_CppunitTest_use_rdb,sc_opencl-1,services))

$(eval $(call gb_CppunitTest_use_configuration,sc_opencl-1))

# vim: set noet sw=4 ts=4:
