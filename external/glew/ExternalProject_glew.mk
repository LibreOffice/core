# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,glew))

$(eval $(call gb_ExternalProject_register_targets,glew,\
	build \
))

ifeq ($(COM),MSC)
$(call gb_ExternalProject_get_state_target,glew,build) :
	$(call gb_ExternalProject_run,build,\
		msbuild.exe glew_shared.vcxproj /p:Platform=$(if $(filter INTEL,$(CPUNAME)),Win32,x64) /p:Configuration=$(if $(MSVC_USE_DEBUG_RUNTIME),Debug,Release) /p:PlatformToolset=v140 \
	,build/vc14) \
	$(call gb_ExternalProject_run,build,\
		msbuild.exe glewinfo.vcxproj /p:Platform=$(if $(filter INTEL,$(CPUNAME)),Win32,x64) /p:Configuration=Release /p:PlatformToolset=v140 \
	,build/vc14)

else

$(call gb_ExternalProject_get_state_target,glew,build) :
	$(call gb_ExternalProject_run,glew,\
		$(if $(ENABLE_DEBUG),STRIP=) LD="$(CC)" \
			$(MAKE) STRIP= glew.lib $(if $(filter DESKTOP,$(BUILD_TYPE)),$(if $(ENABLE_DEBUG),debug)) \
	)

endif

# vim: set noet sw=4 ts=4:
