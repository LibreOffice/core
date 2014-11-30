# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,coinmp))

$(eval $(call gb_ExternalProject_register_targets,coinmp,\
	build \
))

ifeq ($(COM),MSC)
$(call gb_ExternalProject_get_state_target,coinmp,build) :
	$(call gb_ExternalProject_run,build,\
		MSBuild.exe CoinMP.sln /t:Build \
			/p:Configuration=$(if $(MSVC_USE_DEBUG_RUNTIME),Debug,Release) \
			/p:Platform=$(if $(filter INTEL,$(CPUNAME)),Win32,x64) \
			$(if $(filter 110,$(VCVER)),/p:PlatformToolset=$(if $(filter 80,$(WINDOWS_SDK_VERSION)),v110,v110_xp) /p:VisualStudioVersion=11.0) \
			$(if $(filter 120,$(VCVER)),/p:PlatformToolset=v120 /p:VisualStudioVersion=12.0 /ToolsVersion:12.0) \
	,CoinMP/MSVisualStudio/v9)

else
$(call gb_ExternalProject_get_state_target,coinmp,build) :
	+$(call gb_ExternalProject_run,build,\
		./configure COIN_SKIP_PROJECTS="Data/Sample" \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(DISABLE_DYNLOADING),--disable-shared) \
			--enable-dependency-linking F77=unavailable \
		&& $(MAKE) \
	)

endif

# vim: set noet sw=4 ts=4:
