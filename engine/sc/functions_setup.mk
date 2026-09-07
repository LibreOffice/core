# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# common body for the functions test templates, $(1) is the full test name
# and $(2) is the test source file under sc/qa/unit
define sc_functions_test_common

$(eval $(call gb_CppunitTest_CppunitTest,$(1)))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,$(1)))

$(eval $(call gb_CppunitTest_add_exception_objects,$(1), \
    sc/qa/unit/$(2) \
))

$(eval $(call gb_CppunitTest_use_externals,$(1), \
	boost_headers \
	mdds_headers \
        md4c \
	libxml2 \
))

$(eval $(call gb_CppunitTest_use_libraries,$(1), \
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

$(eval $(call gb_CppunitTest_set_include,$(1),\
    -I$(SRCDIR)/sc/source/ui/inc \
    -I$(SRCDIR)/sc/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,$(1),\
    offapi \
    udkapi \
    oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,$(1)))

$(eval $(call gb_CppunitTest_use_vcl,$(1)))

$(eval $(call gb_CppunitTest_use_rdb,$(1),services))

$(eval $(call gb_CppunitTest_use_configuration,$(1)))

endef

# template for functions tests
define sc_functions_test
$(call sc_functions_test_common,sc_$(1)_functions_test,functions_$(1))
endef

# template for functions tests driven by a self-checking OOXML fixture workbook
define sc_ooxml_functions_test
$(call sc_functions_test_common,sc_ooxml_functions_$(1)_test,functions_$(1))
endef

# vim: set noet sw=4 ts=4:
