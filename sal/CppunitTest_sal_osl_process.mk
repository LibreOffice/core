# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sal_osl_process))

$(eval $(call gb_CppunitTest_add_exception_objects,sal_osl_process,\
    sal/qa/osl/process/osl_process \
))

$(eval $(call gb_CppunitTest_use_libraries,sal_osl_process,\
    sal \
	$(gb_UWINAPI) \
))

# Depend on to be tested executable
$(call gb_CppunitTest_get_target,sal_osl_process) : \
	$(call gb_Executable_get_target,osl_process_child)

# vim: set noet sw=4 ts=4:
