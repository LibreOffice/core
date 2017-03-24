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

$(eval $(call gb_CppunitTest_CppunitTest,sw_uiwriter))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_uiwriter, \
    sw/qa/extras/uiwriter/uiwriter \
))

# note: this links msword only for the reason to have a order dependency,
# because "make sw.check" will not see the dependency through services.rdb
$(eval $(call gb_CppunitTest_use_libraries,sw_uiwriter, \
    comphelper \
    cppu \
    cppuhelper \
    editeng \
    msword \
    sal \
    sfx \
    svl \
    svt \
    svxcore \
    sw \
    test \
    unotest \
    vcl \
    tl \
    utl \
))

$(eval $(call gb_CppunitTest_use_externals,sw_uiwriter,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_uiwriter,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/source/uibase/inc \
    -I$(SRCDIR)/sw/qa/extras/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sw_uiwriter))

$(eval $(call gb_CppunitTest_use_ure,sw_uiwriter))
$(eval $(call gb_CppunitTest_use_vcl,sw_uiwriter))

$(eval $(call gb_CppunitTest_use_rdb,sw_uiwriter,services))

$(eval $(call gb_CppunitTest_use_configuration,sw_uiwriter))

$(call gb_CppunitTest_get_target,sw_uiwriter): \
    $(call gb_Library_get_target,textconv_dict)

# vim: set noet sw=4 ts=4:
