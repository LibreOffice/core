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
		msbuild.exe glew_shared.vcxproj /p:Configuration=$(if $(MSVC_USE_DEBUG_RUNTIME),Debug,Release) \
		$(if $(filter 110,$(VCVER)),/p:PlatformToolset=v110 /p:VisualStudioVersion=11.0) \
	,build/vc10)

else

$(call gb_ExternalProject_get_state_target,glew,build) :
	$(call gb_ExternalProject_run,glew,\
		$(if $(filter TRUE,$(ENABLE_DEBUG)),STRIP=) LD="$(CC)" \
			$(MAKE) STRIP= glew.lib $(if $(filter DESKTOP,$(BUILD_TYPE)),$(if $(filter TRUE,$(ENABLE_DEBUG)),debug)) \
	)

endif

# vim: set noet sw=4 ts=4:
