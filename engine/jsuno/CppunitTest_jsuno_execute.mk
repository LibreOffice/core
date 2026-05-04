# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# Copyright the Collabora Office contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,jsuno_execute))

$(eval $(call gb_CppunitTest_add_exception_objects,jsuno_execute, \
    jsuno/qa/unit/execute \
))

$(eval $(call gb_CppunitTest_use_libraries,jsuno_execute, \
    jsuno \
    sal \
))

$(eval $(call gb_CppunitTest_use_sdk_api,jsuno_execute))

$(eval $(call gb_CppunitTest_use_ure,jsuno_execute))

# vim: set noet sw=4 ts=4:
