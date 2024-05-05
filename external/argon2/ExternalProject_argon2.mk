# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,argon2))

$(eval $(call gb_ExternalProject_register_targets,argon2,\
	build \
))

ifeq ($(COM),MSC)

# .vcxproj needs manual SolutionDir to find header; there is a .sln file but it
# expects "x86" where we pass "Win32"

$(call gb_ExternalProject_get_state_target,argon2,build):
	$(call gb_Trace_StartRange,argon2,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		MSBuild.exe vs2015/Argon2OptDll/Argon2OptDll.vcxproj \
			/p:SolutionDir="$(gb_UnpackedTarball_workdir)/argon2/" \
			$(gb_MSBUILD_CONFIG_AND_PLATFORM) \
			/p:PlatformToolset=$(VCTOOLSET) /p:VisualStudioVersion=$(VCVER) /ToolsVersion:Current \
			$(if $(filter 10,$(WINDOWS_SDK_VERSION)),/p:WindowsTargetPlatformVersion=$(UCRTVERSION)) \
		&& MSBuild.exe vs2015/Argon2OptTestCI/Argon2OptTestCI.vcxproj \
			/p:SolutionDir="$(gb_UnpackedTarball_workdir)/argon2/" \
			$(gb_MSBUILD_CONFIG_AND_PLATFORM) \
			/p:PlatformToolset=$(VCTOOLSET) /p:VisualStudioVersion=$(VCVER) /ToolsVersion:Current \
			$(if $(filter 10,$(WINDOWS_SDK_VERSION)),/p:WindowsTargetPlatformVersion=$(UCRTVERSION)) \
		$(if $(CROSS_COMPILING),,&& vs2015/build/Argon2OptTestCI.exe) \
	)
	$(call gb_Trace_EndRange,argon2,EXTERNAL)

else # $(COM)

# clear MAKEFILES because Makefile relies on built-in rules

$(call gb_ExternalProject_get_state_target,argon2,build):
	$(call gb_Trace_StartRange,argon2,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		CFLAGS="$(CFLAGS) $(if $(filter-out WNT MACOSX,$(OS)),-fvisibility=hidden) -fPIC" \
		MAKEFLAGS= $(MAKE) \
			OPTTARGET=$(if $(filter X86_64,$(CPUNAME)),x86-64,forcefail) \
		$(if $(CROSS_COMPILING),,&& $(MAKE) test) \
	)
	$(call gb_Trace_EndRange,argon2,EXTERNAL)

endif # $(COM)

# vim: set noet sw=4 ts=4:
