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

$(eval $(call gb_CppunitTest_CppunitTest,sw_pdf_test))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sw_pdf_test))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_pdf_test, \
    sw/qa/extras/pdf/HybridPdfTest \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_pdf_test, \
    comphelper \
    cppu \
    cppuhelper \
    i18nlangtag \
    msfilter \
    sal \
    sfx \
    sot \
    subsequenttest \
    sw \
    swqahelper \
    svl \
    svt \
    test \
    tl \
    unotest \
    utl \
    vcl \
))

$(eval $(call gb_CppunitTest_use_externals,sw_pdf_test,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_use_executable,sw_pdf_test,xpdfimport))

$(eval $(call gb_CppunitTest_set_include,sw_pdf_test,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/source/uibase/inc \
    -I$(SRCDIR)/sw/source/filter/inc \
    -I$(SRCDIR)/sw/qa/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_pdf_test,\
    udkapi \
    offapi \
    oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_pdf_test))
$(eval $(call gb_CppunitTest_use_vcl,sw_pdf_test))
$(eval $(call gb_CppunitTest_use_rdb,sw_pdf_test,services))
$(eval $(call gb_CppunitTest_use_configuration,sw_pdf_test))

ifneq ($(filter MORE_FONTS,$(BUILD_TYPE)),)
$(eval $(call gb_CppunitTest_set_non_application_font_use,sw_pdf_test,abort))
endif

# vim: set noet sw=4 ts=4:
