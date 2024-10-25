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

$(eval $(call gb_CppunitTest_CppunitTest,editeng_editeng))

$(eval $(call gb_CppunitTest_add_exception_objects,editeng_editeng, \
    editeng/qa/editeng/editeng \
))

$(eval $(call gb_CppunitTest_use_library_objects,editeng_editeng,editeng))

$(eval $(call gb_CppunitTest_use_libraries,editeng_editeng, \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    docmodel \
    i18nlangtag \
    i18nutil \
    lng \
    sal \
    salhelper \
    sax \
    sot \
    sfx \
    svl \
    svt \
    test \
    tk \
    tl \
    ucbhelper \
    unotest \
    utl \
    vcl \
    xo \
))

$(eval $(call gb_CppunitTest_use_externals,editeng_editeng,\
    boost_headers \
    icuuc \
    libxml2 \
	yrs \
))

$(eval $(call gb_CppunitTest_set_include,editeng_editeng,\
    -I$(SRCDIR)/editeng/inc \
    -I$(SRCDIR)/editeng/source/editeng \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,editeng_editeng))

$(eval $(call gb_CppunitTest_use_ure,editeng_editeng))
$(eval $(call gb_CppunitTest_use_vcl,editeng_editeng))

$(eval $(call gb_CppunitTest_use_rdb,editeng_editeng,services))

$(eval $(call gb_CppunitTest_use_configuration,editeng_editeng))

$(eval $(call gb_CppunitTest_use_more_fonts,editeng_editeng))

# vim: set noet sw=4 ts=4:
