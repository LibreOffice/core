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

$(eval $(call gb_CppunitTest_CppunitTest,sw_updateall_object_replacements))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sw_updateall_object_replacements))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_updateall_object_replacements, \
    sw/qa/core/updateall_objectreplacements \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_updateall_object_replacements, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    sfx \
    svxcore \
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

$(eval $(call gb_CppunitTest_use_externals,sw_updateall_object_replacements,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_updateall_object_replacements,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/source/uibase/inc \
    -I$(SRCDIR)/sw/qa/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_updateall_object_replacements,\
    udkapi \
    offapi \
    oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_updateall_object_replacements))
$(eval $(call gb_CppunitTest_use_vcl,sw_updateall_object_replacements))

$(eval $(call gb_CppunitTest_use_rdb,sw_updateall_object_replacements,services))

$(eval $(call gb_CppunitTest_use_custom_headers,sw_updateall_object_replacements,\
    officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,sw_updateall_object_replacements))

$(eval $(call gb_CppunitTest_use_uiconfigs,sw_updateall_object_replacements, \
    modules/swriter \
))

$(eval $(call gb_CppunitTest_use_more_fonts,sw_updateall_object_replacements))

# vim: set noet sw=4 ts=4:
