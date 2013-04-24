# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,cppuhelper_qa_weak))

$(eval $(call gb_CppunitTest_add_exception_objects,cppuhelper_qa_weak,\
    cppuhelper/qa/weak/test_weak \
))

$(eval $(call gb_CppunitTest_use_api,cppuhelper_qa_weak,\
	udkapi \
))

$(eval $(call gb_CppunitTest_use_libraries,cppuhelper_qa_weak,\
	cppu \
    cppuhelper \
	sal \
	$(gb_UWINAPI) \
))

# vim: set noet sw=4 ts=4:
