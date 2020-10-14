# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,framework_loadenv))

$(eval $(call gb_CppunitTest_add_exception_objects,framework_loadenv, \
    framework/qa/cppunit/loadenv \
))

$(eval $(call gb_CppunitTest_use_sdk_api,framework_loadenv))

$(eval $(call gb_CppunitTest_use_libraries,framework_loadenv, \
	comphelper \
	cppu \
	cppuhelper \
	sal \
	salhelper \
	test \
	unotest \
	vcl \
))

$(eval $(call gb_CppunitTest_use_external,framework_loadenv,boost_headers))

$(eval $(call gb_CppunitTest_use_sdk_api,framework_loadenv))

$(eval $(call gb_CppunitTest_use_ure,framework_loadenv))
$(eval $(call gb_CppunitTest_use_vcl,framework_loadenv))

$(eval $(call gb_CppunitTest_use_rdb,framework_loadenv,services))

$(eval $(call gb_CppunitTest_use_configuration,framework_loadenv))

# vim: set noet sw=4 ts=4:
