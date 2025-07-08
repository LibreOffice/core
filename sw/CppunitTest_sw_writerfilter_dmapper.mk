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

$(eval $(call gb_CppunitTest_CppunitTest,sw_writerfilter_dmapper))

$(eval $(call gb_CppunitTest_use_externals,sw_writerfilter_dmapper,\
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_writerfilter_dmapper, \
    sw/qa/writerfilter/dmapper/CellColorHandler \
    sw/qa/writerfilter/dmapper/DomainMapperTableHandler \
    sw/qa/writerfilter/dmapper/DomainMapperTableManager \
    sw/qa/writerfilter/dmapper/DomainMapper \
    sw/qa/writerfilter/dmapper/DomainMapper_Impl \
    sw/qa/writerfilter/dmapper/FontTable \
    sw/qa/writerfilter/dmapper/GraphicImport \
    sw/qa/writerfilter/dmapper/NumberingManager \
    sw/qa/writerfilter/dmapper/TableManager \
    sw/qa/writerfilter/dmapper/TextEffectsHandler \
    sw/qa/writerfilter/dmapper/PropertyMap \
    sw/qa/writerfilter/dmapper/SdtHelper \
    sw/qa/writerfilter/dmapper/SettingsTable \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_writerfilter_dmapper, \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    editeng \
    oox \
    sal \
    subsequenttest \
    test \
    unotest \
    utl \
    tl \
    vcl \
    svl \
    sw \
    swqahelper \
))

$(eval $(call gb_CppunitTest_set_include,sw_writerfilter_dmapper,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/source/uibase/inc \
    -I$(SRCDIR)/sw/qa/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sw_writerfilter_dmapper))

$(eval $(call gb_CppunitTest_use_ure,sw_writerfilter_dmapper))
$(eval $(call gb_CppunitTest_use_vcl,sw_writerfilter_dmapper))

$(eval $(call gb_CppunitTest_use_rdb,sw_writerfilter_dmapper,services))

$(eval $(call gb_CppunitTest_use_custom_headers,sw_writerfilter_dmapper,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,sw_writerfilter_dmapper))

# we need to explicitly depend on library writerfilter because it is not implied
# by a link relation
$(call gb_CppunitTest_get_target,sw_writerfilter_dmapper) : $(call gb_Library_get_target,sw_writerfilter)

# vim: set noet sw=4 ts=4:
