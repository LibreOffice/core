# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,stoc_dump))

$(eval $(call gb_CppunitTest_add_exception_objects,stoc_dump, \
    stoc/test/dump \
))

$(eval $(call gb_CppunitTest_use_internal_api,stoc_dump, \
    cppu_qa_cppumaker_types \
))

$(eval $(call gb_CppunitTest_use_libraries,stoc_dump, \
    cppu \
    cppuhelper \
    sal \
))

$(eval $(call gb_CppunitTest_use_udk_api,stoc_dump))

$(eval $(call gb_CppunitTest_use_ure,stoc_dump))

$(call gb_CppunitTest_get_target,stoc_dump): \
    UNO_TYPES += $(call gb_UnoApiTarget_get_target,cppu_qa_cppumaker_types)

# vim: set noet sw=4 ts=4:
