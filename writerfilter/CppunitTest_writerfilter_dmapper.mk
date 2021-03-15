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

$(eval $(call gb_CppunitTest_CppunitTest,writerfilter_dmapper))

$(eval $(call gb_CppunitTest_use_externals,writerfilter_dmapper,\
	boost_headers \
))

$(eval $(call gb_CppunitTest_add_exception_objects,writerfilter_dmapper, \
    writerfilter/qa/cppunittests/dmapper/CellColorHandler \
    writerfilter/qa/cppunittests/dmapper/DomainMapperTableHandler \
    writerfilter/qa/cppunittests/dmapper/DomainMapper \
    writerfilter/qa/cppunittests/dmapper/DomainMapper_Impl \
    writerfilter/qa/cppunittests/dmapper/GraphicImport \
    writerfilter/qa/cppunittests/dmapper/TextEffectsHandler \
    writerfilter/qa/cppunittests/dmapper/PropertyMap \
))

$(eval $(call gb_CppunitTest_use_libraries,writerfilter_dmapper, \
    basegfx \
    comphelper \
    cppu \
    oox \
    sal \
    test \
    unotest \
    vcl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,writerfilter_dmapper))

$(eval $(call gb_CppunitTest_use_ure,writerfilter_dmapper))
$(eval $(call gb_CppunitTest_use_vcl,writerfilter_dmapper))

$(eval $(call gb_CppunitTest_use_rdb,writerfilter_dmapper,services))

$(eval $(call gb_CppunitTest_use_custom_headers,writerfilter_dmapper,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,writerfilter_dmapper))

# we need to explicitly depend on library writerfilter because it is not implied
# by a link relation
$(call gb_CppunitTest_get_target,writerfilter_dmapper) : $(call gb_Library_get_target,writerfilter)

# vim: set noet sw=4 ts=4:
