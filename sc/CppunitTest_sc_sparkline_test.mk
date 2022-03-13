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

$(eval $(call gb_CppunitTest_CppunitTest,sc_sparkline_test))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sc_sparkline_test))

$(eval $(call gb_CppunitTest_add_exception_objects,sc_sparkline_test, \
    sc/qa/unit/SparklineImportExportTest \
))

$(eval $(call gb_CppunitTest_use_libraries,sc_sparkline_test, \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    salhelper \
    sax \
    sc \
    scqahelper \
    sfx \
    subsequenttest \
    test \
    tl \
    unotest \
    vcl \
))

$(eval $(call gb_CppunitTest_set_include,sc_sparkline_test,\
    -I$(SRCDIR)/sc/source/ui/inc \
    -I$(SRCDIR)/sc/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sc_sparkline_test,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sc_sparkline_test))

$(eval $(call gb_CppunitTest_use_ure,sc_sparkline_test))
$(eval $(call gb_CppunitTest_use_vcl,sc_sparkline_test))

$(eval $(call gb_CppunitTest_use_rdb,sc_sparkline_test,services))

$(eval $(call gb_CppunitTest_use_components,sc_sparkline_test))

$(eval $(call gb_CppunitTest_use_configuration,sc_sparkline_test))

# vim: set noet sw=4 ts=4:
