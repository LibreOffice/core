# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sc_copypaste))

$(eval $(call gb_CppunitTest_add_exception_objects,sc_copypaste, \
    sc/qa/unit/copy_paste_test \
))

$(eval $(call gb_CppunitTest_use_externals,sc_copypaste, \
	boost_headers \
	mdds_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_use_libraries,sc_copypaste, \
    comphelper \
    cppu \
    cppuhelper \
    i18nlangtag \
    sal \
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

$(eval $(call gb_CppunitTest_set_include,sc_copypaste,\
    -I$(SRCDIR)/sc/source/ui/inc \
    -I$(SRCDIR)/sc/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sc_copypaste))

$(eval $(call gb_CppunitTest_use_ure,sc_copypaste))
$(eval $(call gb_CppunitTest_use_vcl,sc_copypaste))

$(eval $(call gb_CppunitTest_use_rdb,sc_copypaste,services))

$(eval $(call gb_CppunitTest_use_configuration,sc_copypaste))

# vim: set noet sw=4 ts=4:
