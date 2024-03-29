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

$(eval $(call gb_CppunitTest_CppunitTest,sc_parallelism))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sc_parallelism))

$(eval $(call gb_CppunitTest_add_exception_objects,sc_parallelism, \
    sc/qa/unit/parallelism \
))

$(eval $(call gb_CppunitTest_use_externals,sc_parallelism, \
    boost_headers \
    mdds_headers \
    libxml2 \
    $(call gb_Helper_optional,OPENCL, \
        clew) \
))

$(eval $(call gb_CppunitTest_use_libraries,sc_parallelism, \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    i18nlangtag \
    sal \
    sax \
    sc \
    scqahelper \
    sfx \
    subsequenttest \
    svl \
    svx \
    svxcore \
    test \
    tl \
    unotest \
    utl \
    vcl \
))

$(eval $(call gb_CppunitTest_set_include,sc_parallelism,\
	-I$(SRCDIR)/sc/source/ui/inc \
	-I$(SRCDIR)/sc/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sc_parallelism,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_custom_headers,sc_parallelism,\
    officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sc_parallelism))

$(eval $(call gb_CppunitTest_use_ure,sc_parallelism))

$(eval $(call gb_CppunitTest_use_vcl,sc_parallelism))

$(eval $(call gb_CppunitTest_use_rdb,sc_parallelism,services))

$(eval $(call gb_CppunitTest_use_components,sc_parallelism))

$(eval $(call gb_CppunitTest_use_configuration,sc_parallelism))

$(eval $(call gb_CppunitTest_add_arguments,sc_parallelism, \
    -env:arg-env=$(gb_Helper_LIBRARY_PATH_VAR)"$$$${$(gb_Helper_LIBRARY_PATH_VAR)+=$$$$$(gb_Helper_LIBRARY_PATH_VAR)}" \
))

# vim: set noet sw=4 ts=4:
