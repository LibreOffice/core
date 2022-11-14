# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# template for functions tests
define sc_functions_test

$(eval $(call gb_CppunitTest_CppunitTest,sc_$(1)_functions_test))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sc_$(1)_functions_test))

$(eval $(call gb_CppunitTest_add_exception_objects,sc_$(1)_functions_test, \
    sc/qa/unit/functions_$(1) \
))

$(eval $(call gb_CppunitTest_use_externals,sc_$(1)_functions_test, \
	boost_headers \
	mdds_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_use_libraries,sc_$(1)_functions_test, \
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

$(eval $(call gb_CppunitTest_set_include,sc_$(1)_functions_test,\
    -I$(SRCDIR)/sc/source/ui/inc \
    -I$(SRCDIR)/sc/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sc_$(1)_functions_test,\
    offapi \
    udkapi \
    oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sc_$(1)_functions_test))

$(eval $(call gb_CppunitTest_use_vcl,sc_$(1)_functions_test))

$(eval $(call gb_CppunitTest_use_rdb,sc_$(1)_functions_test,services))

$(eval $(call gb_CppunitTest_use_configuration,sc_$(1)_functions_test))

endef

# vim: set noet sw=4 ts=4:
