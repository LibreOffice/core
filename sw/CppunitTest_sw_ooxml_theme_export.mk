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

$(eval $(call gb_CppunitTest_CppunitTest,sw_ooxml_theme_export))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sw_ooxml_theme_export))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_ooxml_theme_export, \
    sw/qa/extras/ooxmlexport/ooxml_ThemeExport \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_ooxml_theme_export, \
	$(sw_ooxmlexport_libraries) \
))

$(eval $(call gb_CppunitTest_use_externals,sw_ooxml_theme_export,\
	boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_ooxml_theme_export,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
	-I$(SRCDIR)/sw/qa/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_ooxml_theme_export,\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_ooxml_theme_export))
$(eval $(call gb_CppunitTest_use_vcl,sw_ooxml_theme_export))

$(eval $(call gb_CppunitTest_use_rdb,sw_ooxml_theme_export,services))

$(eval $(call gb_CppunitTest_use_configuration,sw_ooxml_theme_export))

$(eval $(call gb_CppunitTest_use_packages,sw_ooxml_theme_export,\
	oox_generated \
))

ifneq ($(filter MORE_FONTS,$(BUILD_TYPE)),)
$(eval $(call gb_CppunitTest_set_non_application_font_use,sw_ooxml_theme_export,abort))
endif

# vim: set noet sw=4 ts=4:
