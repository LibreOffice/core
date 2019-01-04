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

$(eval $(call gb_CppunitTest_CppunitTest,sw_unowriter))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_unowriter, \
    sw/qa/extras/unowriter/unowriter \
))

# note: this links msword only for the reason to have a order dependency,
# because "make sw.check" will not see the dependency through services.rdb
$(eval $(call gb_CppunitTest_use_libraries,sw_unowriter, \
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
    tk \
    utl \
))

$(eval $(call gb_CppunitTest_use_externals,sw_unowriter,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_unowriter,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/source/uibase/inc \
    -I$(SRCDIR)/sw/qa/extras/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_unowriter,\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_unowriter))
$(eval $(call gb_CppunitTest_use_vcl,sw_unowriter))

$(eval $(call gb_CppunitTest_use_rdb,sw_unowriter,services))

$(eval $(call gb_CppunitTest_use_configuration,sw_unowriter))

$(eval $(call gb_CppunitTest_use_uiconfigs,sw_unowriter, \
    modules/swriter \
))

$(call gb_CppunitTest_get_target,sw_unowriter): \
    $(call gb_Library_get_target,textconv_dict)

$(eval $(call gb_CppunitTest_use_more_fonts,sw_unowriter))

# vim: set noet sw=4 ts=4:
