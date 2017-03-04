# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sfx2_classification))

$(eval $(call gb_CppunitTest_add_exception_objects,sfx2_classification, \
    sfx2/qa/cppunit/test_classification \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sfx2_classification))

$(eval $(call gb_CppunitTest_use_libraries,sfx2_classification, \
	comphelper \
	cppu \
	cppuhelper \
	test \
	unotest \
	vcl \
    sal \
    sfx \
))

$(eval $(call gb_CppunitTest_use_external,sfx2_classification,boost_headers))

$(eval $(call gb_CppunitTest_use_sdk_api,sfx2_classification))

$(eval $(call gb_CppunitTest_use_ure,sfx2_classification))
$(eval $(call gb_CppunitTest_use_vcl,sfx2_classification))

$(eval $(call gb_CppunitTest_use_rdb,sfx2_classification,services))

$(eval $(call gb_CppunitTest_use_instdir_configuration,sfx2_classification))

$(eval $(call gb_CppunitTest_use_uiconfigs,sfx2_classification,\
	svx \
))

$(eval $(call gb_CppunitTest_use_packages,sfx2_classification,\
	sfx2_classification \
))

# vim: set noet sw=4 ts=4:
