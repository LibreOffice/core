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
    sc/qa/unit/SparklineTest \
))

$(eval $(call gb_CppunitTest_use_externals,sc_sparkline_test, \
    boost_headers \
    mdds_headers \
    libxml2 \
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
    utl \
    vcl \
))

$(eval $(call gb_CppunitTest_use_externals,sc_sparkline_test,\
    boost_headers \
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

$(eval $(call gb_CppunitTest_add_arguments,sc_sparkline_test, \
    -env:arg-env=$(gb_Helper_LIBRARY_PATH_VAR)"$$$${$(gb_Helper_LIBRARY_PATH_VAR)+=$$$$$(gb_Helper_LIBRARY_PATH_VAR)}" \
))

# vim: set noet sw=4 ts=4:
