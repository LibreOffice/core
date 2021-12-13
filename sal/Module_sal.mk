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
	$(if $(CROSS_COMPILING),,$(if $(filter TRUE,$(DISABLE_DYNLOADING)),,Executable_cppunittester)) \
	$(if $(filter $(OS),ANDROID), \
		Library_lo-bootstrap) \
	Library_sal \
    $(call gb_CondSalTextEncodingLibrary,Library_sal_textenc) \
))

ifneq (,$(filter DESKTOP,$(BUILD_TYPE)))

$(eval $(call gb_Module_add_targets,sal,\
	Executable_osl_process_child \
))

$(eval $(call gb_Module_add_check_targets,sal,\
	$(if $(filter TRUE,$(DISABLE_DYNLOADING)),,CppunitTest_Module_DLL) \
	$(if $(filter WNT,$(OS)),CppunitTest_sal_comtools) \
	$(if $(filter WNT,$(OS)),CppunitTest_sal_retry_if_failed) \
	CppunitTest_sal_osl_security \
	CppunitTest_sal_osl \
	CppunitTest_sal_rtl \
	CppunitTest_sal_types \
	$(if $(COM_IS_CLANG),$(if $(COMPILER_EXTERNAL_TOOL)$(COMPILER_PLUGIN_TOOL),, \
	    CompilerTest_sal_rtl_oustring)) \
))

endif

# vim: set noet sw=4 ts=4:
