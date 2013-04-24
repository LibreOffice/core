# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# This is not really a test. Only a dynamic library which is loaded by
# sal_osl_module unit test library.

$(eval $(call gb_CppunitTest_CppunitTest,Module_DLL))

$(eval $(call gb_CppunitTest_set_visibility_default,Module_DLL))

$(eval $(call gb_CppunitTest_add_exception_objects,Module_DLL,\
    sal/qa/osl/module/osl_Module_DLL \
))

$(eval $(call gb_CppunitTest_use_libraries,Module_DLL,\
    sal \
	$(gb_UWINAPI) \
))

# vim: set noet sw=4 ts=4:
