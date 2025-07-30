# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,test_a11y))

$(eval $(call gb_CppunitTest_add_exception_objects,test_a11y, \
	test/qa/cppunit/dialog \
))

$(eval $(call gb_CppunitTest_use_libraries,test_a11y, \
	sal \
	comphelper \
	cppu \
	cppuhelper \
	subsequenttest \
	test \
))

$(eval $(call gb_CppunitTest_use_sdk_api,test_a11y))
$(eval $(call gb_CppunitTest_use_rdb,test_a11y,services))
$(eval $(call gb_CppunitTest_use_ure,test_a11y))
$(eval $(call gb_CppunitTest_use_vcl,test_a11y))

$(eval $(call gb_CppunitTest_use_instdir_configuration,test_a11y))
$(eval $(call gb_CppunitTest_use_common_configuration,test_a11y))

$(eval $(call gb_CppunitTest_use_packages,test_a11y, \
    postprocess_images \
))

# vim: set noet sw=4 ts=4:
