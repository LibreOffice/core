# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sc_macros_test))

$(eval $(call gb_CppunitTest_use_externals,sc_macros_test, \
    boost_headers \
    mdds_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sc_macros_test))

$(eval $(call gb_CppunitTest_add_exception_objects,sc_macros_test, \
    sc/qa/extras/macros-test \
))

$(eval $(call gb_CppunitTest_use_libraries,sc_macros_test, \
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
    $(call gb_Helper_optional,SCRIPTING, \
        vbahelper) \
    vcl \
    xo \
))

$(eval $(call gb_CppunitTest_set_include,sc_macros_test,\
    -I$(SRCDIR)/sc/source/ui/inc \
    -I$(SRCDIR)/sc/inc \
    -I$(SRCDIR)/sc/qa/unit \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sc_macros_test,\
    offapi \
    oovbaapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,sc_macros_test))
$(eval $(call gb_CppunitTest_use_vcl,sc_macros_test))

$(eval $(call gb_CppunitTest_use_rdb,sc_macros_test,services))

$(eval $(call gb_CppunitTest_use_configuration,sc_macros_test))

# vim: set noet sw=4 ts=4:
