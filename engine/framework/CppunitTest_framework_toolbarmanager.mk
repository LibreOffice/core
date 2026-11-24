# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,framework_toolbarmanager))

$(eval $(call gb_CppunitTest_add_exception_objects,framework_toolbarmanager, \
    framework/qa/cppunit/toolbarmanager \
))

$(eval $(call gb_CppunitTest_use_sdk_api,framework_toolbarmanager))

$(eval $(call gb_CppunitTest_use_libraries,framework_toolbarmanager, \
	comphelper \
	cppu \
	sal \
	subsequenttest \
	test \
	tk \
	tl \
	unotest \
	utl \
	vcl \
))

$(eval $(call gb_CppunitTest_use_ure,framework_toolbarmanager))
$(eval $(call gb_CppunitTest_use_vcl,framework_toolbarmanager))

$(eval $(call gb_CppunitTest_use_rdb,framework_toolbarmanager,services))

$(eval $(call gb_CppunitTest_use_uiconfigs,framework_toolbarmanager, \
    modules/swriter \
))

$(eval $(call gb_CppunitTest_use_instdir_configuration,framework_toolbarmanager))
$(eval $(call gb_CppunitTest_use_common_configuration,framework_toolbarmanager))

# vim: set noet sw=4 ts=4:
