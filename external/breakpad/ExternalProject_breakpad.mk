# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,breakpad))

$(eval $(call gb_ExternalProject_register_targets,breakpad,\
	build \
))


ifeq ($(COM),MSC)

$(call gb_ExternalProject_get_state_target,breakpad,build) :
	$(call gb_ExternalProject_run,build,\
		msbuild.exe breakpad.vcxproj /p:Platform=$(if $(filter INTEL,$(CPUNAME)),Win32,x64) \
			/p:Configuration=$(if $(MSVC_USE_DEBUG_RUNTIME),Debug,Release) \
			$(if $(filter 120,$(VCVER)),/p:PlatformToolset=v120 /p:VisualStudioVersion=12.0 /ToolsVersion:12.0) \
			$(if $(filter 140,$(VCVER)),/p:PlatformToolset=v140 /p:VisualStudioVersion=14.0 /ToolsVersion:14.0) \
			'/p:AdditionalIncludeDirectories=$(subst $(WHITESPACE),;,$(subst /,\,$(strip $(libgltf_AdditionalIncludes))))' \
			/p:AdditionalLibraryDirectories=$(if $(SYSTEM_GLEW),,"$(subst /,\,$(call gb_UnpackedTarball_get_dir,glew))\lib\$(if $(MSVC_USE_DEBUG_RUNTIME),Debug,Release)\Win32") \
	,build/win32)

else # !ifeq($(COM),MSC)

$(call gb_ExternalProject_get_state_target,breakpad,build) :
	$(call gb_ExternalProject_run,build,\
		./configure \
		&& $(MAKE) \
	)

endif

# vim: set noet sw=4 ts=4:
