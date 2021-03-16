# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,xmlscript_cppunit))

$(eval $(call gb_CppunitTest_add_exception_objects,xmlscript_cppunit, \
    xmlscript/qa/cppunit/test \
))

$(eval $(call gb_CppunitTest_use_sdk_api,xmlscript_cppunit))
$(eval $(call gb_CppunitTest_use_ure,xmlscript_cppunit))
$(eval $(call gb_CppunitTest_use_vcl,xmlscript_cppunit))
$(eval $(call gb_CppunitTest_use_rdb,xmlscript_cppunit,services))

$(eval $(call gb_CppunitTest_use_custom_headers,xmlscript_cppunit,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_externals,xmlscript_cppunit,\
    libxml2 \
))

$(eval $(call gb_CppunitTest_use_configuration,xmlscript_cppunit))

$(eval $(call gb_CppunitTest_use_libraries,xmlscript_cppunit, \
	comphelper \
	cppu \
	cppuhelper \
	fwk \
	sal \
	test \
    unotest \
    utl \
    xmlscript \
))

# vim: set noet sw=4 ts=4:
