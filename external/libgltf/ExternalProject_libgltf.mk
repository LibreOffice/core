# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libgltf))

$(eval $(call gb_ExternalProject_use_autoconf,libgltf,build))

$(eval $(call gb_ExternalProject_register_targets,libgltf,\
	build \
))

$(eval $(call gb_ExternalProject_use_externals,libgltf,\
	boost_headers \
	glew \
	glm_headers \
))


ifeq ($(COM),MSC)

AdditionalIncludes :=

ifeq ($(SYSTEM_BOOST),)
AdditionalIncludes += "$(call gb_UnpackedTarball_get_dir,boost)"
AdditionalIncludes += "$(SRCDIR)/config_host"
endif

ifeq ($(SYSTEM_GLEW),)
AdditionalIncludes += "$(call gb_UnpackedTarball_get_dir,glew)/include"
endif

ifeq ($(SYSTEM_GLM),)
AdditionalIncludes += "$(call gb_UnpackedTarball_get_dir,glm)"
endif

empty :=
space := $(empty) $(empty)

$(call gb_ExternalProject_get_state_target,libgltf,build) :
	$(call gb_ExternalProject_run,build,\
		msbuild.exe libgltf.vcxproj \
			/p:Configuration=$(if $(MSVC_USE_DEBUG_RUNTIME),Debug,Release) \
			$(if $(filter 110,$(VCVER)),/p:PlatformToolset=$(if $(filter 80,$(WINDOWS_SDK_VERSION)),v110,v110_xp) \
			/p:VisualStudioVersion=11.0) \
			'/p:AdditionalIncludeDirectories=$(subst $(space),;,$(subst /,\,$(strip $(AdditionalIncludes))))' \
			/p:AdditionalLibraryDirectories=$(if $(SYSTEM_GLEW),,"$(subst /,\,$(call gb_UnpackedTarball_get_dir,glew))\lib\$(if $(MSVC_USE_DEBUG_RUNTIME),Debug,Release)\Win32") \
	,build/win32)

else

$(call gb_ExternalProject_get_state_target,libgltf,build) :
	$(call gb_ExternalProject_run,build,\
		export PKG_CONFIG="" \
		&& ./configure \
			--disable-debug \
			--disable-werror \
			BOOST_CFLAGS="$(if $(SYSTEM_BOOST),$(BOOST_CPPFLAGS),-I$(call gb_UnpackedTarball_get_dir,boost))" \
			GLEW_CFLAGS="$(if $(SYSTEM_GLEW),$(GLEW_CFLAGS),-I$(call gb_UnpackedTarball_get_dir,glew)/include)" \
			GLM_CFLAGS="$(if $(SYSTEM_GLM),$(GLM_CFLAGS),-I$(call gb_UnpackedTarball_get_dir,glm))" \
		&& $(MAKE) \
	)

endif

# vim: set noet sw=4 ts=4:
