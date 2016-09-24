# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,sal))

$(eval $(call gb_Module_add_targets,sal,\
	$(if $(CROSS_COMPILING),,Executable_cppunittester) \
	$(if $(filter $(OS),ANDROID), \
		Library_lo-bootstrap) \
	Library_sal \
	$(if $(filter $(OS),ANDROID),, \
		Library_sal_textenc) \
	$(if $(filter $(OS),WNT), \
		Library_uwinapi) \
	StaticLibrary_salcpprt \
))

ifneq (,$(filter DESKTOP,$(BUILD_TYPE)))

$(eval $(call gb_Module_add_targets,sal,\
		Executable_osl_process_child \
))

$(eval $(call gb_Module_add_check_targets,sal,\
	$(if $(filter $(OS),WNT),,CppunitTest_sal_osl_file) \
	$(if $(filter TRUE,$(DISABLE_DYNLOADING)),,CppunitTest_Module_DLL) \
	$(if $(filter TRUE,$(DISABLE_DYNLOADING)),,CppunitTest_sal_osl_module) \
	$(if $(filter-out MSC-120,$(COM)-$(VCVER)),CppunitTest_sal_osl_security) \
	CppunitTest_sal_bytesequence \
	CppunitTest_sal_osl_condition \
	CppunitTest_sal_osl_getsystempathfromfileurl \
	CppunitTest_sal_osl_mutex \
	CppunitTest_sal_osl_old_test_file \
	CppunitTest_sal_osl_pipe \
	CppunitTest_sal_osl_process \
	CppunitTest_sal_osl_profile \
	CppunitTest_sal_osl_setthreadname \
	CppunitTest_sal_osl_thread \
	CppunitTest_sal_rtl_alloc \
	CppunitTest_sal_rtl_bootstrap \
	CppunitTest_sal_rtl_cipher \
	CppunitTest_sal_rtl_crc32 \
	CppunitTest_sal_rtl_digest \
	CppunitTest_sal_rtl_doublelock \
	CppunitTest_sal_rtl_locale \
	CppunitTest_sal_rtl_math \
	CppunitTest_sal_rtl_ostringbuffer \
	CppunitTest_sal_rtl_oustring \
	CppunitTest_sal_rtl_oustringbuffer \
	CppunitTest_sal_rtl_process \
	CppunitTest_sal_rtl_random \
	CppunitTest_sal_rtl_ref \
	CppunitTest_sal_rtl_strings \
	CppunitTest_sal_rtl_textenc \
	CppunitTest_sal_rtl_uri \
	CppunitTest_sal_rtl_uuid \
	CppunitTest_sal_tcwf \
	CppunitTest_sal_types \
))

endif

# vim: set noet sw=4 ts=4:
