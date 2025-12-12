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

$(eval $(call gb_CppunitTest_CppunitTest,sw_cjk))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sw_cjk))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_cjk, \
    sw/qa/extras/cjk/cjk \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_cjk, \
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

$(eval $(call gb_CppunitTest_use_externals,sw_cjk,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_cjk,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/source/uibase/inc \
    -I$(SRCDIR)/sw/qa/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_system_win32_libs,sw_cjk,\
    ole32 \
))

$(eval $(call gb_CppunitTest_use_api,sw_cjk,\
    udkapi \
    offapi \
    oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_cjk))
$(eval $(call gb_CppunitTest_use_vcl,sw_cjk))

$(eval $(call gb_CppunitTest_use_rdb,sw_cjk,services))

$(eval $(call gb_CppunitTest_use_configuration,sw_cjk))

$(eval $(call gb_CppunitTest_add_arguments,sw_cjk, \
    -env:arg-env=$(gb_Helper_LIBRARY_PATH_VAR)"$$$${$(gb_Helper_LIBRARY_PATH_VAR)+=$$$$$(gb_Helper_LIBRARY_PATH_VAR)}" \
))

$(eval $(call gb_CppunitTest_use_custom_headers,sw_cjk,\
    officecfg/registry \
))

# vim: set noet sw=4 ts=4:
