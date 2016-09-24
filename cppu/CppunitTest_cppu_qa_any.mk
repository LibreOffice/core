# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,cppu_qa_any))

$(eval $(call gb_CppunitTest_add_exception_objects,cppu_qa_any, \
	cppu/qa/test_any \
))

$(eval $(call gb_CppunitTest_use_external,cppu_qa_any,boost_headers))

$(eval $(call gb_CppunitTest_use_libraries,cppu_qa_any, \
	cppu \
	sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_internal_comprehensive_api,cppu_qa_any, \
	cppu_qa_types \
	udkapi \
))

# vim: set noet sw=4 ts=4:
