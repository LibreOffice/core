# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#*************************************************************************

$(eval $(call gb_CppunitTest_CppunitTest,sc_cellcursor))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sc_cellcursor))

$(eval $(call gb_CppunitTest_add_exception_objects,sc_cellcursor, \
    sc/qa/unit/tiledrendering/CellCursorTest \
))

$(eval $(call gb_CppunitTest_use_libraries,sc_cellcursor, \
    comphelper \
    cppu \
    cppuhelper \
    docmodel \
    editeng \
    sal \
    sfx \
    sofficeapp \
    sot \
    svl \
    svt \
    svxcore \
    sc \
    scui \
    scqahelper \
    subsequenttest \
    test \
    unotest \
    $(call gb_Helper_optional,SCRIPTING, \
        vbahelper) \
    vcl \
    tl \
    utl \
    $(if $(filter -fsanitize=vptr,$(gb_CXX)),salhelper) \
))

$(eval $(call gb_CppunitTest_use_externals,sc_cellcursor,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sc_cellcursor,\
    -I$(SRCDIR)/sc/source/ui/inc \
    -I$(SRCDIR)/sc/inc \
	-I$(SRCDIR)/sc/qa/unit/helper \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sc_cellcursor))
$(eval $(call gb_CppunitTest_use_api,sc_cellcursor,oovbaapi))

$(eval $(call gb_CppunitTest_use_ure,sc_cellcursor))
$(eval $(call gb_CppunitTest_use_vcl,sc_cellcursor))

$(eval $(call gb_CppunitTest_use_rdb,sc_cellcursor,services))

$(eval $(call gb_CppunitTest_use_configuration,sc_cellcursor))

$(eval $(call gb_CppunitTest_add_arguments,sc_cellcursor, \
    -env:arg-env=$(gb_Helper_LIBRARY_PATH_VAR)"$$$${$(gb_Helper_LIBRARY_PATH_VAR)+=$$$$$(gb_Helper_LIBRARY_PATH_VAR)}" \
))

$(eval $(call gb_CppunitTest_use_uiconfigs,sc_cellcursor, \
    modules/scalc \
    sfx \
    svt \
))

# vim: set noet sw=4 ts=4:
