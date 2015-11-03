# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,filter_pcd_test))

$(eval $(call gb_CppunitTest_use_external,filter_pcd_test,boost_headers))

$(eval $(call gb_CppunitTest_add_exception_objects,filter_pcd_test, \
    filter/qa/cppunit/filters-pcd-test \
))

$(eval $(call gb_CppunitTest_use_libraries,filter_pcd_test, \
    gie \
	sal \
	test \
	tl \
	unotest \
	vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_api,filter_pcd_test,\
    udkapi \
    offapi \
))

$(eval $(call gb_CppunitTest_use_ure,filter_pcd_test))
$(eval $(call gb_CppunitTest_use_vcl,filter_pcd_test))

$(eval $(call gb_CppunitTest_use_components,filter_pcd_test,\
    configmgr/source/configmgr \
))

$(eval $(call gb_CppunitTest_use_configuration,filter_pcd_test))

# vim: set noet sw=4 ts=4:
