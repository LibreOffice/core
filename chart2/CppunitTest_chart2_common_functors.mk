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

$(eval $(call gb_CppunitTest_CppunitTest,chart2_common_functors))

$(eval $(call gb_CppunitTest_add_exception_objects,chart2_common_functors, \
    chart2/qa/unit/common_functor_test \
))

$(eval $(call gb_CppunitTest_add_defs,chart2_common_functors,\
    -DOOO_DLLIMPLEMENTATION_CHARTTOOLS \
))

$(eval $(call gb_CppunitTest_use_externals,chart2_common_functors, \
    boost_headers \
))

$(eval $(call gb_CppunitTest_use_libraries,chart2_common_functors, \
    cppu \
    cppuhelper \
    sal \
    salhelper \
))

$(eval $(call gb_CppunitTest_set_include,chart2_common_functors,\
    -I$(SRCDIR)/chart2/inc \
    -I$(SRCDIR)/chart2/source/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,chart2_common_functors))

$(eval $(call gb_CppunitTest_use_ure,chart2_common_functors))

# vim: set noet sw=4 ts=4:
