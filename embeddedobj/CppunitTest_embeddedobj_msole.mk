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

$(eval $(call gb_CppunitTest_CppunitTest,embeddedobj_msole))

$(eval $(call gb_CppunitTest_use_externals,embeddedobj_msole,\
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_add_exception_objects,embeddedobj_msole, \
    embeddedobj/qa/cppunit/msole \
))

$(eval $(call gb_CppunitTest_use_libraries,embeddedobj_msole, \
    comphelper \
    cppu \
    embobj \
    sal \
    test \
    unotest \
    utl \
    vcl \
    tl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,embeddedobj_msole))

$(eval $(call gb_CppunitTest_use_ure,embeddedobj_msole))
$(eval $(call gb_CppunitTest_use_vcl,embeddedobj_msole))

$(eval $(call gb_CppunitTest_use_rdb,embeddedobj_msole,services))

$(eval $(call gb_CppunitTest_use_custom_headers,embeddedobj_msole,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,embeddedobj_msole))

# vim: set noet sw=4 ts=4:
