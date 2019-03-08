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

$(eval $(call gb_CppunitTest_CppunitTest,sw_filters_test))

$(eval $(call gb_CppunitTest_use_externals,sw_filters_test,\
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_filters_test, \
    sw/qa/core/filters-test \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_filters_test, \
    sw \
    sfx \
    svl \
    svt \
    sot \
	test \
    tl \
    ucbhelper \
    unotest \
    utl \
    vcl \
    i18nlangtag \
    comphelper \
    cppu \
    cppuhelper \
    sal \
))

$(eval $(call gb_CppunitTest_set_include,sw_filters_test,\
    -I$(SRCDIR)/sw/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_filters_test,\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_filters_test))
$(eval $(call gb_CppunitTest_use_vcl,sw_filters_test))

$(eval $(call gb_CppunitTest_use_rdb,sw_filters_test,services))

$(eval $(call gb_CppunitTest_use_configuration,sw_filters_test))

$(call gb_CppunitTest_get_target,sw_filters_test): \
    $(call gb_Package_get_target,extras_tplwizdesktop)

# vim: set noet sw=4 ts=4:
