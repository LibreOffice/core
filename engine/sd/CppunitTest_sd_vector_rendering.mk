# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sd_vector_rendering))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sd_vector_rendering))

$(eval $(call gb_CppunitTest_add_exception_objects,sd_vector_rendering, \
    sd/qa/unit/vector/VectorRenderingTest \
    sd/qa/unit/vector/VectorPrimitiveReferenceTest \
))

$(eval $(call gb_CppunitTest_use_libraries,sd_vector_rendering, \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    drawinglayer \
    drawinglayercore \
    editeng \
    sal \
    sfx \
    sofficeapp \
    subsequenttest \
    svl \
    svt \
    svxcore \
    sd \
    sdqahelper \
    test \
    unotest \
    vcl \
    tl \
    utl \
))

$(eval $(call gb_CppunitTest_use_externals,sd_vector_rendering,\
    boost_headers \
    frozen \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sd_vector_rendering,\
    -I$(SRCDIR)/sd/inc \
    -I$(SRCDIR)/sd/source/ui/inc \
    -I$(SRCDIR)/sd/qa/unit \
    -I$(SRCDIR)/sd/qa/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sd_vector_rendering))

$(eval $(call gb_CppunitTest_use_ure,sd_vector_rendering))
$(eval $(call gb_CppunitTest_use_vcl,sd_vector_rendering))

$(eval $(call gb_CppunitTest_use_rdb,sd_vector_rendering,services))

$(eval $(call gb_CppunitTest_use_configuration,sd_vector_rendering))

$(eval $(call gb_CppunitTest_use_uiconfigs,sd_vector_rendering, \
    modules/simpress \
    svx \
))

$(eval $(call gb_CppunitTest_add_arguments,sd_vector_rendering, \
    -env:arg-env=$(gb_Helper_LIBRARY_PATH_VAR)"$$$${$(gb_Helper_LIBRARY_PATH_VAR)+=$$$$$(gb_Helper_LIBRARY_PATH_VAR)}" \
))

# vim: set noet sw=4 ts=4:
