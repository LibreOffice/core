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

$(eval $(call gb_CppunitTest_CppunitTest,sd_core))

$(eval $(call gb_CppunitTest_use_externals,sd_core,\
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_add_exception_objects,sd_core, \
    sd/qa/core/core \
))

$(eval $(call gb_CppunitTest_use_libraries,sd_core, \
    comphelper \
    cppu \
    cppuhelper \
    sd \
    sal \
    subsequenttest \
    test \
    unotest \
    utl \
    tl \
))

$(eval $(call gb_CppunitTest_set_include,sd_core,\
    -I$(SRCDIR)/sd/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sd_core))

$(eval $(call gb_CppunitTest_use_ure,sd_core))
$(eval $(call gb_CppunitTest_use_vcl,sd_core))

$(eval $(call gb_CppunitTest_use_rdb,sd_core,services))

$(eval $(call gb_CppunitTest_use_custom_headers,sd_core,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,sd_core))

$(eval $(call gb_CppunitTest_add_arguments,sd_core, \
    -env:arg-env=$(gb_Helper_LIBRARY_PATH_VAR)"$$$${$(gb_Helper_LIBRARY_PATH_VAR)+=$$$$$(gb_Helper_LIBRARY_PATH_VAR)}" \
))

# vim: set noet sw=4 ts=4:
