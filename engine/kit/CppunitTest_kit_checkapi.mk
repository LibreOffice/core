# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,kit_checkapi))

$(eval $(call gb_CppunitTest_add_cxxflags,kit_checkapi, \
    $(gb_CXX03FLAGS) \
))

$(eval $(call gb_CppunitTest_add_exception_objects,kit_checkapi, \
    kit/qa/unit/checkapi \
))
$(eval $(call gb_CppunitTest_add_cobjects,kit_checkapi,\
	kit/qa/unit/compile_test \
))

$(eval $(call gb_CppunitTest_set_external_code,kit_checkapi))

ifeq ($(OS),LINUX)
$(eval $(call gb_CppunitTest_add_libs,kit_checkapi, \
    -ldl \
))
endif

# vim: set noet sw=4 ts=4:
