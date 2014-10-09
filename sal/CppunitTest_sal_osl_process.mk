# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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

$(eval $(call gb_CppunitTest_use_externals,sal_osl_process,\
    valgrind \
))

$(eval $(call gb_CppunitTest_use_executable,sal_osl_process,osl_process_child))

# vim: set noet sw=4 ts=4:
