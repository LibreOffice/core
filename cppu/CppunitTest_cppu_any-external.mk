# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,cppu_any-external))

$(eval $(call gb_CppunitTest_add_exception_objects,cppu_any-external, \
    cppu/qa/any-external \
))

$(eval $(call gb_CppunitTest_set_external_code,cppu_any-external))

$(eval $(call gb_CppunitTest_use_libraries,cppu_any-external, \
    cppu \
    sal \
))

$(eval $(call gb_CppunitTest_use_udk_api,cppu_any-external))

# vim: set noet sw=4 ts=4:
