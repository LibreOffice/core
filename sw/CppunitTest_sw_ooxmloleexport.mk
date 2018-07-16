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

$(eval $(call gb_CppunitTest_CppunitTest,sw_ooxmloleexport))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_ooxmloleexport, \
    sw/qa/extras/ooxmlexport/ooxmloleexport \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_ooxmloleexport, \
    $(sw_ooxmlexport_libraries) \
))

$(eval $(call gb_CppunitTest_use_externals,sw_ooxmloleexport,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_ooxmloleexport,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/qa/extras/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sw_ooxmloleexport,))

$(eval $(call gb_CppunitTest_use_api,sw_ooxmloleexport,\
    udkapi \
    offapi \
    oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_ooxmloleexport))
$(eval $(call gb_CppunitTest_use_vcl,sw_ooxmloleexport))

$(eval $(call gb_CppunitTest_use_components,sw_ooxmloleexport,\
    $(sw_ooxmlexport_components) \
    filter/source/storagefilterdetect/storagefd \
))

$(eval $(call gb_CppunitTest_use_configuration,sw_ooxmloleexport))

$(eval $(call gb_CppunitTest_use_uiconfigs,sw_ooxmloleexport,\
    modules/swriter \
))

$(eval $(call gb_CppunitTest_use_packages,sw_ooxmloleexport,\
    oox_customshapes \
    oox_generated \
))

$(call gb_CppunitTest_get_target,sw_ooxmloleexport) : $(call gb_Library_get_target,iti)


# vim: set noet sw=4 ts=4:
