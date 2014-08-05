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

libgltf_AdditionalIncludes :=

ifeq ($(SYSTEM_BOOST),)
libgltf_AdditionalIncludes += "$(call gb_UnpackedTarball_get_dir,boost)"
libgltf_AdditionalIncludes += "$(BUILDDIR)/config_$(gb_Side)"
endif

ifeq ($(SYSTEM_GLEW),)
libgltf_AdditionalIncludes += "$(call gb_UnpackedTarball_get_dir,glew)/include"
endif

ifeq ($(SYSTEM_GLM),)
libgltf_AdditionalIncludes += "$(call gb_UnpackedTarball_get_dir,glm)"
endif

$(call gb_ExternalProject_get_state_target,libgltf,build) :
	$(call gb_ExternalProject_run,build,\
		msbuild.exe libgltf.vcxproj \
			/p:Configuration=$(if $(MSVC_USE_DEBUG_RUNTIME),Debug,Release) \
			$(if $(filter 110,$(VCVER)),/p:PlatformToolset=$(if $(filter 80,$(WINDOWS_SDK_VERSION)),v110,v110_xp) \
				/p:VisualStudioVersion=11.0) \
			$(if $(filter 100,$(VCVER)),/p:PlatformToolset=v100 \
				/p:VisualStudioVersion=10.0) \
			'/p:AdditionalIncludeDirectories=$(subst $(WHITESPACE),;,$(subst /,\,$(strip $(libgltf_AdditionalIncludes))))' \
			/p:AdditionalLibraryDirectories=$(if $(SYSTEM_GLEW),,"$(subst /,\,$(call gb_UnpackedTarball_get_dir,glew))\lib\$(if $(MSVC_USE_DEBUG_RUNTIME),Debug,Release)\Win32") \
	,build/win32)

else

libgltf_CPPFLAGS :=
ifneq (,$(gb_ENABLE_DBGUTIL))
	libgltf_CPPFLAGS += -D_GLIBCXX_DEBUG
endif

$(call gb_ExternalProject_get_state_target,libgltf,build) :
	$(call gb_ExternalProject_run,build,\
		CPPFLAGS='$(libgltf_CPPFLAGS)' \
		export PKG_CONFIG="" \
		&& ./configure \
			--with-pic \
			--disable-debug \
			--disable-werror \
			BOOST_CFLAGS="$(if $(SYSTEM_BOOST),$(BOOST_CPPFLAGS),-I$(call gb_UnpackedTarball_get_dir,boost)) -I$(BUILDDIR)/config_$(gb_Side)" \
			GLEW_CFLAGS="$(if $(SYSTEM_GLEW),$(GLEW_CFLAGS),-I$(call gb_UnpackedTarball_get_dir,glew)/include)" \
			GLM_CFLAGS="$(if $(SYSTEM_GLM),$(GLM_CFLAGS),-I$(call gb_UnpackedTarball_get_dir,glm))" \
		&& $(MAKE) \
	)

endif

# vim: set noet sw=4 ts=4:
