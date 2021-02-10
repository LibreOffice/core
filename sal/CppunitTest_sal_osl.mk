# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sal_osl))

$(eval $(call gb_CppunitTest_add_exception_objects,sal_osl,\
	sal/qa/osl/condition/osl_Condition \
	sal/qa/osl/file/osl_File \
	sal/qa/osl/file/osl_old_test_file \
	sal/qa/osl/file/test_cpy_wrt_file \
	sal/qa/osl/getsystempathfromfileurl/test-getsystempathfromfileurl \
	$(if $(DISABLE_DYNLOADING),,sal/qa/osl/module/osl_Module) \
	sal/qa/osl/mutex/osl_Mutex \
	sal/qa/osl/pipe/osl_Pipe \
	sal/qa/osl/process/osl_process \
	sal/qa/osl/process/osl_Thread \
	sal/qa/osl/profile/osl_old_testprofile \
	sal/qa/osl/setthreadname/test-setthreadname \
))

$(eval $(call gb_CppunitTest_use_libraries,sal_osl,\
	sal \
	tl \
))

# the test uses the library created by Module_DLL
$(call gb_CppunitTest_get_target,sal_osl) : \
	$(call gb_LinkTarget_get_target,$(call gb_CppunitTest_get_linktarget,Module_DLL))

$(eval $(call gb_CppunitTest_use_executable,sal_osl,osl_process_child))

$(eval $(call gb_CppunitTest_use_externals,sal_osl,\
	boost_headers \
	valgrind \
))

# vim: set noet sw=4 ts=4:
