# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,framework_CheckXTitle))

$(eval $(call gb_CppunitTest_add_exception_objects,framework_CheckXTitle, \
    framework/qa/cppunit/CheckXTitle \
))

$(eval $(call gb_CppunitTest_use_sdk_api,framework_CheckXTitle))

$(eval $(call gb_CppunitTest_use_libraries,framework_CheckXTitle, \
	comphelper \
	cppu \
	cppuhelper \
	sal \
	salhelper \
	subsequenttest \
	test \
	unotest \
	utl \
	tl \
	vcl \
))

$(eval $(call gb_CppunitTest_use_external,framework_CheckXTitle,boost_headers))

$(eval $(call gb_CppunitTest_use_sdk_api,framework_CheckXTitle))

$(eval $(call gb_CppunitTest_use_ure,framework_CheckXTitle))
$(eval $(call gb_CppunitTest_use_vcl,framework_CheckXTitle))

$(eval $(call gb_CppunitTest_use_rdb,framework_CheckXTitle,services))

$(eval $(call gb_CppunitTest_use_configuration,framework_CheckXTitle))

# vim: set noet sw=4 ts=4:
