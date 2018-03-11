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
$(eval $(call gb_CppunitTest_CppunitTest,sc_check_data_pilot_table))

$(eval $(call gb_CppunitTest_add_exception_objects,sc_check_data_pilot_table, \
        sc/qa/extras/check_data_pilot_table \
))

$(eval $(call gb_CppunitTest_use_libraries,sc_check_data_pilot_table, \
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
        sb \
        sc \
        scqahelper \
        sfx \
        sot \
        subsequenttest \
        svl \
        svt \
        svx \
        svxcore \
        test \
        tk \
        tl \
        ucbhelper \
        unotest \
        utl \
        vbahelper \
        vcl \
        xo \
))

$(eval $(call gb_CppunitTest_set_include,sc_check_data_pilot_table,\
        -I$(SRCDIR)/sc/source/ui/inc \
        -I$(SRCDIR)/sc/inc \
        $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sc_check_data_pilot_table,\
        offapi \
        udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,sc_check_data_pilot_table))
$(eval $(call gb_CppunitTest_use_vcl,sc_check_data_pilot_table))

$(eval $(call gb_CppunitTest_use_components,sc_check_data_pilot_table,\
    $(sc_unoapi_common_components) \
))

$(eval $(call gb_CppunitTest_use_configuration,sc_check_data_pilot_table))

# vim: set noet sw=4 ts=4:
