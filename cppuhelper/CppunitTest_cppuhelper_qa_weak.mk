# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2011 Matúš Kukan <matus.kukan@gmail.com> (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

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

$(eval $(call gb_CppunitTest_add_standard_system_libs,cppuhelper_qa_weak))

# vim: set noet sw=4 ts=4:
