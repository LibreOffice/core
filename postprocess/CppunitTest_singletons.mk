# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,singletons))

$(eval $(call gb_CppunitTest_add_exception_objects,singletons, \
    postprocess/qa/singletons \
))

$(eval $(call gb_CppunitTest_use_libraries,singletons, \
	comphelper \
	cppu \
	cppuhelper \
	sal \
	test \
	unotest \
	vcl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,singletons))
$(eval $(call gb_CppunitTest_use_api,singletons,oovbaapi))

$(eval $(call gb_CppunitTest_use_ure,singletons))
$(eval $(call gb_CppunitTest_use_vcl,singletons))

$(eval $(call gb_CppunitTest_use_rdb,singletons,services))

$(eval $(call gb_CppunitTest_use_configuration,singletons))

# vim: set noet sw=4 ts=4:
