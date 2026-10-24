# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# Copyright the Collabora Office contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sc_vector_rendering))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sc_vector_rendering))

$(eval $(call gb_CppunitTest_add_exception_objects,sc_vector_rendering, \
    sc/qa/unit/tiledrendering/VectorRenderingTest \
))

$(eval $(call gb_CppunitTest_use_libraries,sc_vector_rendering, \
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

$(eval $(call gb_CppunitTest_use_externals,sc_vector_rendering,\
    boost_headers \
    mdds_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sc_vector_rendering,\
    -I$(SRCDIR)/sc/source/ui/inc \
    -I$(SRCDIR)/sc/inc \
	-I$(SRCDIR)/sc/qa/unit/helper \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sc_vector_rendering))
$(eval $(call gb_CppunitTest_use_api,sc_vector_rendering,oovbaapi))

$(eval $(call gb_CppunitTest_use_ure,sc_vector_rendering))
$(eval $(call gb_CppunitTest_use_vcl,sc_vector_rendering))

$(eval $(call gb_CppunitTest_use_rdb,sc_vector_rendering,services))

$(eval $(call gb_CppunitTest_use_configuration,sc_vector_rendering))

$(eval $(call gb_CppunitTest_add_arguments,sc_vector_rendering, \
    -env:arg-env=$(gb_Helper_LIBRARY_PATH_VAR)"$$$${$(gb_Helper_LIBRARY_PATH_VAR)+=$$$$$(gb_Helper_LIBRARY_PATH_VAR)}" \
))

$(eval $(call gb_CppunitTest_use_uiconfigs,sc_vector_rendering, \
    modules/scalc \
    sfx \
    svt \
))

# vim: set noet sw=4 ts=4:
