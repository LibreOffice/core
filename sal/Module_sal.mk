# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
#       Bjoern Michaelsen, Canonical Ltd. <bjoern.michaelsen@canonical.com>
# Portions created by the Initial Developer are Copyright (C) 2010 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Module_Module,sal))

$(eval $(call gb_Module_add_targets,sal,\
	CustomTarget_generated \
	CustomTarget_sal_allheaders \
	Executable_cppunittester \
	$(if $(filter $(OS),ANDROID), \
		Library_lo-bootstrap) \
	Library_sal \
	$(if $(filter $(OS),ANDROID),, \
		Library_sal_textenc) \
	$(if $(filter $(OS),WNT), \
		Library_uwinapi) \
	Package_inc \
	Package_generated \
	StaticLibrary_salcpprt \
))

ifneq (,$(filter DESKTOP,$(BUILD_TYPE)))
$(eval $(call gb_Module_add_targets,sal,\
		Executable_osl_process_child \
		CppunitTest_sal_osl_process \
))
endif

$(eval $(call gb_Module_add_check_targets,sal,\
	CppunitTest_Module_DLL \
	CppunitTest_sal_bytesequence \
	CppunitTest_sal_checkapi \
	CppunitTest_sal_osl_condition \
	$(if $(filter $(OS),WNT),, \
		CppunitTest_sal_osl_file) \
	CppunitTest_sal_osl_module \
	CppunitTest_sal_osl_old_test_file \
	CppunitTest_sal_osl_security \
	CppunitTest_sal_osl_thread \
	CppunitTest_sal_rtl_alloc \
	CppunitTest_sal_rtl_cipher \
	CppunitTest_sal_rtl_crc32 \
	CppunitTest_sal_rtl_doublelock \
	CppunitTest_sal_rtl_locale \
	CppunitTest_sal_rtl_ostringbuffer \
	CppunitTest_sal_rtl_oustringbuffer \
	CppunitTest_sal_rtl_strings \
	CppunitTest_sal_rtl_uuid \
	CppunitTest_sal_tcwf \
	CppunitTest_sal_types \
))

# these were added when gbuild'izing
$(eval $(call gb_Module_add_check_targets,sal,\
	CppunitTest_sal_osl_mutex \
	CppunitTest_sal_osl_profile \
	CppunitTest_sal_osl_setthreadname \
	CppunitTest_sal_rtl_math \
))

# CppunitTest_sal_osl_pipe has circular dependency on unotest
# $(eval $(call gb_Module_add_subsequentcheck_targets,sal,\
	CppunitTest_sal_osl_pipe \
))

# error when building test-getsystempathfromfileurl.cxx \
	CppunitTest_sal_osl_getsystempathfromfileurl \

# vim: set noet sw=4 ts=4:
