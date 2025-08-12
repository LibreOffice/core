# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sw_embedded_fonts))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sw_embedded_fonts))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_embedded_fonts, \
    sw/qa/extras/embedded_fonts/embedded_fonts \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_embedded_fonts, \
    comphelper \
    editeng \
    cppu \
    cppuhelper \
    sal \
    svt \
    sfx \
    subsequenttest \
    svl \
    sw \
    swqahelper \
    test \
    unotest \
    vcl \
    tl \
    utl \
))

$(eval $(call gb_CppunitTest_use_externals,sw_embedded_fonts,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_embedded_fonts,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/source/uibase/inc \
    -I$(SRCDIR)/sw/qa/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_system_win32_libs,sw_embedded_fonts,\
    ole32 \
))

$(eval $(call gb_CppunitTest_use_api,sw_embedded_fonts,\
    udkapi \
    offapi \
    oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_embedded_fonts))
$(eval $(call gb_CppunitTest_use_vcl,sw_embedded_fonts))

$(eval $(call gb_CppunitTest_use_rdb,sw_embedded_fonts,services))

$(eval $(call gb_CppunitTest_use_configuration,sw_embedded_fonts))

$(eval $(call gb_CppunitTest_add_arguments,sw_embedded_fonts, \
    -env:arg-env=$(gb_Helper_LIBRARY_PATH_VAR)"$$$${$(gb_Helper_LIBRARY_PATH_VAR)+=$$$$$(gb_Helper_LIBRARY_PATH_VAR)}" \
))

$(eval $(call gb_CppunitTest_use_custom_headers,sw_embedded_fonts,\
    officecfg/registry \
))

# Explicitly allow non-application fonts
$(eval $(call gb_CppunitTest_set_non_application_font_use,sw_embedded_fonts,allow))

# vim: set noet sw=4 ts=4:
