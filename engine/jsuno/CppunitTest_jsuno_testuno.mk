# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,jsuno_testuno))

$(eval $(call gb_CppunitTest_add_exception_objects,jsuno_testuno, \
    jsuno/qa/unit/testuno \
))

$(eval $(call gb_CppunitTest_use_libraries,jsuno_testuno, \
    jsuno \
    sal \
))

$(eval $(call gb_CppunitTest_use_components,jsuno_testuno, \
    unotest/source/testuno/testuno \
))

$(eval $(call gb_CppunitTest_use_sdk_api,jsuno_testuno))

$(eval $(call gb_CppunitTest_use_ure,jsuno_testuno))

# vim: set noet sw=4 ts=4:
