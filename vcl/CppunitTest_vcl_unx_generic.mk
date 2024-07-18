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

$(eval $(call gb_CppunitTest_CppunitTest,vcl_unx_generic))

$(eval $(call gb_CppunitTest_use_externals,vcl_unx_generic,\
    boost_headers \
    harfbuzz \
))

$(eval $(call gb_CppunitTest_set_include,vcl_unx_generic,\
    -I$(SRCDIR)/vcl/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_add_exception_objects,vcl_unx_generic, \
    vcl/qa/unx/generic/generic \
))

$(eval $(call gb_CppunitTest_use_libraries,vcl_unx_generic, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    subsequenttest \
    test \
    tl \
    unotest \
    utl \
    vcl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,vcl_unx_generic))

$(eval $(call gb_CppunitTest_use_ure,vcl_unx_generic))
$(eval $(call gb_CppunitTest_use_vcl,vcl_unx_generic))

$(eval $(call gb_CppunitTest_use_rdb,vcl_unx_generic,services))

$(eval $(call gb_CppunitTest_use_custom_headers,vcl_unx_generic,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,vcl_unx_generic))

# Note that this is intentionally 'deny' and not 'abort', so we allow font fallback (we want to
# test that code), but we want predictable results, so not considering system fonts.
$(eval $(call gb_CppunitTest_set_non_application_font_use,vcl_unx_generic,deny))

$(eval $(call gb_CppunitTest_use_more_fonts,vcl_unx_generic))

# vim: set noet sw=4 ts=4:
