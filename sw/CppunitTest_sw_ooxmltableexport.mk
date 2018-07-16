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

$(eval $(call gb_CppunitTest_CppunitTest,sw_ooxmltableexport))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_ooxmltableexport, \
    sw/qa/extras/ooxmlexport/ooxmltableexport \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_ooxmltableexport, \
    $(sw_ooxmlexport_libraries) \
))

$(eval $(call gb_CppunitTest_use_externals,sw_ooxmltableexport,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_ooxmltableexport,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/qa/extras/inc \
    $$(INCLUDE) \
))

ifeq ($(OS),WNT)
$(eval $(call gb_CppunitTest_add_cxxflags,sw_ooxmltableexport,\
    -bigobj \
))
endif

$(eval $(call gb_CppunitTest_use_sdk_api,sw_ooxmltableexport,))

$(eval $(call gb_CppunitTest_use_api,sw_ooxmltableexport,\
    udkapi \
    offapi \
    oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_ooxmltableexport))
$(eval $(call gb_CppunitTest_use_vcl,sw_ooxmltableexport))

$(eval $(call gb_CppunitTest_use_components,sw_ooxmltableexport,\
    $(sw_ooxmlexport_components) \
    filter/source/storagefilterdetect/storagefd \
))

$(eval $(call gb_CppunitTest_use_configuration,sw_ooxmltableexport))

$(eval $(call gb_CppunitTest_use_uiconfigs,sw_ooxmltableexport,\
    modules/swriter \
))

$(eval $(call gb_CppunitTest_use_packages,sw_ooxmltableexport,\
    oox_customshapes \
    oox_generated \
))

$(call gb_CppunitTest_get_target,sw_ooxmltableexport) : $(call gb_Library_get_target,iti)


# vim: set noet sw=4 ts=4:
