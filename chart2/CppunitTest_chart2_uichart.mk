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

$(eval $(call gb_CppunitTest_CppunitTest,chart2_uichart))

$(eval $(call gb_CppunitTest_use_externals,chart2_uichart, \
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_add_exception_objects,chart2_uichart, \
    chart2/qa/extras/uichart \
))

$(eval $(call gb_CppunitTest_use_libraries,chart2_uichart, \
    comphelper \
    cppu \
    cppuhelper \
    i18nlangtag \
    sal \
    sc \
    sfx \
    subsequenttest \
    svl \
    svl \
    svx \
    svxcore \
    test \
    tl \
    unotest \
    utl \
    vcl \
))

$(eval $(call gb_CppunitTest_set_include,chart2_uichart,\
	-I$(SRCDIR)/chart2/qa/extras \
    -I$(SRCDIR)/chart2/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,chart2_uichart))

$(eval $(call gb_CppunitTest_use_ure,chart2_uichart))
$(eval $(call gb_CppunitTest_use_vcl,chart2_uichart))

$(eval $(call gb_CppunitTest_use_rdb,chart2_uichart,services))

$(eval $(call gb_CppunitTest_use_configuration,chart2_uichart))

# vim: set noet sw=4 ts=4:
