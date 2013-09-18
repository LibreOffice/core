# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,cppu_test_cppumaker))

$(eval $(call gb_CppunitTest_add_exception_objects,cppu_test_cppumaker, \
	cppu/qa/cppumaker/test_cppumaker \
))

$(eval $(call gb_CppunitTest_use_external,cppu_test_cppumaker,boost_headers))

$(eval $(call gb_CppunitTest_use_libraries,cppu_test_cppumaker, \
	cppu \
	sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_internal_comprehensive_api,cppu_test_cppumaker, \
	cppu_qa_cppumaker_types \
	udkapi \
))

# vim: set noet sw=4 ts=4:
