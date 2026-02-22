# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,framework_accelerators))

$(eval $(call gb_CppunitTest_add_exception_objects,framework_accelerators, \
    framework/qa/cppunit/accelerators \
))

$(eval $(call gb_CppunitTest_use_libraries,framework_accelerators, \
	comphelper \
	cppu \
	cppuhelper \
	fwk \
	sal \
	subsequenttest \
	utl \
	tl \
	test \
	unotest \
))

$(eval $(call gb_CppunitTest_use_externals,framework_accelerators, \
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_use_sdk_api,framework_accelerators))

$(eval $(call gb_CppunitTest_use_ure,framework_accelerators))

$(eval $(call gb_CppunitTest_use_vcl,framework_accelerators))

$(eval $(call gb_CppunitTest_use_rdb,framework_accelerators,services))

$(eval $(call gb_CppunitTest_use_configuration,framework_accelerators))

# vim: set noet sw=4 ts=4:
