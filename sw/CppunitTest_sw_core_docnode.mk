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

$(eval $(call gb_CppunitTest_CppunitTest,sw_core_docnode))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sw_core_docnode))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_core_docnode, \
    sw/qa/core/docnode/docnode \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_core_docnode, \
    comphelper \
    cppu \
    cppuhelper \
    editeng \
    sal \
    sfx \
    sw \
    swqahelper \
    test \
    unotest \
    utl \
    vcl \
    svt \
    tl \
    svl \
))

$(eval $(call gb_CppunitTest_use_externals,sw_core_docnode,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_core_docnode,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/source/uibase/inc \
    -I$(SRCDIR)/sw/qa/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_core_docnode,\
    udkapi \
    offapi \
    oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_core_docnode))
$(eval $(call gb_CppunitTest_use_vcl,sw_core_docnode))

$(eval $(call gb_CppunitTest_use_rdb,sw_core_docnode,services))

$(eval $(call gb_CppunitTest_use_custom_headers,sw_core_docnode,\
    officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,sw_core_docnode))

$(eval $(call gb_CppunitTest_use_uiconfigs,sw_core_docnode, \
    modules/swriter \
))

$(eval $(call gb_CppunitTest_use_more_fonts,sw_core_docnode))

# vim: set noet sw=4 ts=4:
