# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,starmath_import))

$(eval $(call gb_CppunitTest_set_include,starmath_import,\
    $$(INCLUDE) \
    -I$(SRCDIR)/starmath/inc \
))

$(eval $(call gb_CppunitTest_use_externals,starmath_import, \
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_use_sdk_api,starmath_import))

$(eval $(call gb_CppunitTest_add_exception_objects,starmath_import,\
    starmath/qa/cppunit/test_import \
    starmath/qa/extras/mmlimport-test \
))

$(eval $(call gb_CppunitTest_use_libraries,starmath_import,\
    comphelper \
    cppu \
    cppuhelper \
    editeng \
    i18nlangtag \
    i18nutil \
    msfilter \
    oox \
    sal \
    salhelper \
    sax \
    sfx \
    sm \
    smd \
    sot \
    subsequenttest \
    svl \
    svt \
    svx \
    svxcore \
    test \
    tk \
    tl \
    unotest \
    unoxml \
    utl \
    vcl \
    xo \
))

$(eval $(call gb_CppunitTest_use_ure,starmath_import))
$(eval $(call gb_CppunitTest_use_vcl,starmath_import))

$(eval $(call gb_CppunitTest_use_rdb,starmath_import,services))

$(eval $(call gb_CppunitTest_use_configuration,starmath_import))

# vim: set noet sw=4 ts=4:
