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

$(eval $(call gb_CppunitTest_CppunitTest,filter_dialogs_test))

$(eval $(call gb_CppunitTest_add_exception_objects,filter_dialogs_test, \
    filter/qa/unit/filter-dialogs-test \
))

$(eval $(call gb_CppunitTest_use_sdk_api,filter_dialogs_test))

$(eval $(call gb_CppunitTest_set_include,filter_dialogs_test,\
    -I$(SRCDIR)/filter/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_libraries,filter_dialogs_test, \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    drawinglayer \
    editeng \
    i18nlangtag \
    i18nutil \
    msfilter \
    oox \
    sal \
    salhelper \
    sax \
    sfx \
    sot \
    svl \
    svt \
    test \
    tl \
    tk \
    ucbhelper \
    unotest \
    utl \
    vcl \
    xo \
    $(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_external,filter_dialogs_test,boost_headers))

$(eval $(call gb_CppunitTest_use_sdk_api,filter_dialogs_test))

$(eval $(call gb_CppunitTest_use_ure,filter_dialogs_test))
$(eval $(call gb_CppunitTest_use_vcl_non_headless_with_windows,filter_dialogs_test))

$(eval $(call gb_CppunitTest_use_rdb,filter_dialogs_test,services))

# $(eval $(call gb_CppunitTest_use_configuration,filter_dialogs_test))
$(eval $(call gb_CppunitTest_use_instdir_configuration,filter_dialogs_test))

# vim: set noet sw=4 ts=4:
