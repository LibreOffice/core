# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,twain_dsm))

$(eval $(call gb_ExternalProject_use_unpacked,twain_dsm,twain_dsm))

$(eval $(call gb_ExternalProject_register_targets,twain_dsm,\
	build \
))

ifeq ($(OS),WNT)

$(call gb_ExternalProject_get_state_target,twain_dsm,build) :
	$(call gb_ExternalProject_run,build,\
		MSBuild.exe visual_studio/TWAIN_DSM_VS2015.sln /t:Build \
			/p:Configuration=$(if $(MSVC_USE_DEBUG_RUNTIME),Debug,Release) \
			/p:Platform=$(if $(filter INTEL,$(CPUNAME)),Win32,x64) \
			/p:OutDir=../PCBuild/out/ /p:IntDir=../PCBuild/obj/ \
			$(if $(filter 140,$(VCVER)),/p:PlatformToolset=v140 /p:VisualStudioVersion=14.0 /ToolsVersion:14.0) \
			$(if $(filter 150,$(VCVER)),/p:PlatformToolset=v141 /p:VisualStudioVersion=15.0 /ToolsVersion:15.0) \
			$(if $(filter 150-10,$(VCVER)-$(WINDOWS_SDK_VERSION)),/p:WindowsTargetPlatformVersion=$(UCRTVERSION)) \
	)

endif

# vim: set noet sw=4 ts=4:
