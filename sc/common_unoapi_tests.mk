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

# template for unoapi tests
define sc_unoapi_common

$(eval $(call gb_CppunitTest_CppunitTest,sc_$(1)))

$(eval $(call gb_CppunitTest_use_external,sc_$(1),boost_headers))

$(eval $(call gb_Library_use_common_precompiled_header,sc_$(1)))

$(eval $(call gb_CppunitTest_add_exception_objects,sc_$(1), \
    sc/qa/extras/sc$(1) \
))

$(eval $(call gb_CppunitTest_use_libraries,sc_$(1), \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    salhelper \
    sc \
    subsequenttest \
    test \
    unotest \
    utl \
    tl \
    vcl \
))

$(eval $(call gb_CppunitTest_set_include,sc_$(1),\
    -I$(SRCDIR)/sc/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sc_$(1),\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,sc_$(1)))
$(eval $(call gb_CppunitTest_use_vcl,sc_$(1)))

$(eval $(call gb_CppunitTest_use_rdb,sc_$(1),services))

$(eval $(call gb_CppunitTest_use_configuration,sc_$(1)))
endef

# vim: set noet sw=4 ts=4:
