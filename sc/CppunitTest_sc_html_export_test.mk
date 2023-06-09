# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sc_html_export_test))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sc_html_export_test))

$(eval $(call gb_CppunitTest_add_exception_objects,sc_html_export_test, \
    sc/qa/extras/htmlexporttest \
))

$(eval $(call gb_CppunitTest_use_externals,sc_html_export_test, \
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_use_libraries,sc_html_export_test, \
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

$(eval $(call gb_CppunitTest_set_include,sc_html_export_test,\
    -I$(SRCDIR)/sc/source/ui/inc \
    -I$(SRCDIR)/sc/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sc_html_export_test,\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_CppunitTest_use_packages,sc_html_export_test, \
    filter_xhtml \
    filter_xslt \
))

$(eval $(call gb_CppunitTest_use_ure,sc_html_export_test))
$(eval $(call gb_CppunitTest_use_vcl,sc_html_export_test))

$(eval $(call gb_CppunitTest_use_rdb,sc_html_export_test,services))

$(eval $(call gb_CppunitTest_use_configuration,sc_html_export_test))

# vim: set noet sw=4 ts=4:
