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

$(eval $(call gb_CppunitTest_CppunitTest,sd_import_tests_smartart))

$(eval $(call gb_CppunitTest_use_externals,sd_import_tests_smartart,\
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_add_exception_objects,sd_import_tests_smartart, \
    sd/qa/unit/import-tests-smartart \
))

$(eval $(call gb_CppunitTest_use_libraries,sd_import_tests_smartart, \
	$(call gb_Helper_optional,AVMEDIA,avmedia) \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    drawinglayer \
    editeng \
    for \
    forui \
    i18nlangtag \
    msfilter \
    oox \
    sal \
    salhelper \
    sax \
    sd \
    sfx \
    sot \
    svl \
    svt \
    svx \
    svxcore \
    test \
    tl \
    tk \
    ucbhelper \
    unotest \
    utl \
    vcl \
    xo \
))

$(eval $(call gb_CppunitTest_set_include,sd_import_tests_smartart,\
    -I$(SRCDIR)/sd/source/ui/inc \
    -I$(SRCDIR)/sd/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sd_import_tests_smartart))

$(eval $(call gb_CppunitTest_use_ure,sd_import_tests_smartart))
$(eval $(call gb_CppunitTest_use_vcl,sd_import_tests_smartart))

$(eval $(call gb_CppunitTest_use_rdb,sd_import_tests_smartart,services))

$(eval $(call gb_CppunitTest_use_configuration,sd_import_tests_smartart))

$(eval $(call gb_CppunitTest_use_packages,sd_import_tests_smartart,\
	oox_customshapes \
))

# vim: set noet sw=4 ts=4:
