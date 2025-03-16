# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,vcl_a11y))

$(eval $(call gb_CppunitTest_add_exception_objects,vcl_a11y, \
	vcl/qa/cppunit/a11y/widgetaccessibilitytest \
))

$(eval $(call gb_CppunitTest_use_libraries,vcl_a11y, \
	sal \
	cppu \
	subsequenttest \
	test \
	vcl \
))

$(eval $(call gb_CppunitTest_use_api,vcl_a11y,\
	offapi \
	udkapi \
))

$(eval $(call gb_CppunitTest_use_sdk_api,vcl_a11y))
$(eval $(call gb_CppunitTest_use_rdb,vcl_a11y,services))
$(eval $(call gb_CppunitTest_use_ure,vcl_a11y))
$(eval $(call gb_CppunitTest_use_vcl,vcl_a11y))

$(eval $(call gb_CppunitTest_use_instdir_configuration,vcl_a11y))
$(eval $(call gb_CppunitTest_use_common_configuration,vcl_a11y))

# vim: set noet sw=4 ts=4:
