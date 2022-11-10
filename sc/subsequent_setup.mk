# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# template for subsequent tests
define sc_subsequent_test

$(eval $(call gb_CppunitTest_CppunitTest,sc_subsequent_$(1)))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sc_subsequent_$(1)))

$(eval $(call gb_CppunitTest_add_exception_objects,sc_subsequent_$(1), \
    sc/qa/unit/subsequent_$(1) \
))

$(eval $(call gb_CppunitTest_use_externals,sc_subsequent_$(1), \
	boost_headers \
	mdds_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_use_libraries,sc_subsequent_$(1), \
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

$(eval $(call gb_CppunitTest_set_include,sc_subsequent_$(1),\
    -I$(SRCDIR)/sc/source/ui/inc \
    -I$(SRCDIR)/sc/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sc_subsequent_$(1),\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sc_subsequent_$(1)))
$(eval $(call gb_CppunitTest_use_vcl,sc_subsequent_$(1)))

$(eval $(call gb_CppunitTest_use_rdb,sc_subsequent_$(1),services))

$(eval $(call gb_CppunitTest_use_custom_headers,sc_subsequent_$(1),\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,sc_subsequent_$(1)))

$(eval $(call gb_CppunitTest_use_packages,sc_subsequent_$(1), \
    oox_generated \
))

$(eval $(call gb_CppunitTest_use_more_fonts,sc_subsequent_$(1)))

$(eval $(call gb_CppunitTest_add_arguments,sc_subsequent_$(1), \
    -env:arg-env=$(gb_Helper_LIBRARY_PATH_VAR)"$$$${$(gb_Helper_LIBRARY_PATH_VAR)+=$$$$$(gb_Helper_LIBRARY_PATH_VAR)}" \
))

endef

# vim: set noet sw=4 ts=4:
