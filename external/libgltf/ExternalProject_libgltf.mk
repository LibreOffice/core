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
endif

ifeq ($(SYSTEM_GLEW),)
libgltf_AdditionalIncludes += "$(call gb_UnpackedTarball_get_dir,glew)/include"
endif

ifeq ($(SYSTEM_GLM),)
libgltf_AdditionalIncludes += "$(call gb_UnpackedTarball_get_dir,glm)"
endif

$(call gb_ExternalProject_get_state_target,libgltf,build) :
	$(call gb_ExternalProject_run,build,\
		msbuild.exe libgltf.vcxproj /p:Platform=$(if $(filter INTEL,$(CPUNAME)),Win32,x64) \
			/p:Configuration=$(if $(MSVC_USE_DEBUG_RUNTIME),Debug,Release) \
			$(if $(filter 120,$(VCVER)),/p:PlatformToolset=v120 /p:VisualStudioVersion=12.0 /ToolsVersion:12.0) \
			$(if $(filter 140,$(VCVER)),/p:PlatformToolset=v140 /p:VisualStudioVersion=14.0 /ToolsVersion:14.0) \
			'/p:AdditionalIncludeDirectories=$(subst $(WHITESPACE),;,$(subst /,\,$(strip $(libgltf_AdditionalIncludes))))' \
			/p:AdditionalLibraryDirectories=$(if $(SYSTEM_GLEW),,"$(subst /,\,$(call gb_UnpackedTarball_get_dir,glew))\lib\$(if $(MSVC_USE_DEBUG_RUNTIME),Debug,Release)\Win32") \
	,build/win32)

else # !ifeq($(COM),MSC)

libgltf_CPPFLAGS=$(CPPFLAGS)
ifneq (,$(filter ANDROID DRAGONFLY FREEBSD IOS LINUX NETBSD OPENBSD,$(OS)))
ifneq (,$(gb_ENABLE_DBGUTIL))
libgltf_CPPFLAGS+=-D_GLIBCXX_DEBUG
endif
endif

$(call gb_ExternalProject_get_state_target,libgltf,build) :
	$(call gb_ExternalProject_run,build,\
		export PKG_CONFIG="" \
		&& ./configure \
			--disable-shared \
			--enable-static \
			--with-pic \
			$(if $(ENABLE_DEBUG),--enable-debug,--disable-debug) \
			--disable-werror \
			BOOST_CFLAGS="$(if $(SYSTEM_BOOST),$(BOOST_CPPFLAGS),-I$(call gb_UnpackedTarball_get_dir,boost))" \
			GLEW_CFLAGS="$(if $(SYSTEM_GLEW),$(GLEW_CFLAGS),-I$(call gb_UnpackedTarball_get_dir,glew)/include) -DGLEW_NO_GLU" \
			GLM_CFLAGS="$(if $(SYSTEM_GLM),$(GLM_CFLAGS),-I$(call gb_UnpackedTarball_get_dir,glm))" \
			$(if $(libgltf_CPPFLAGS),CPPFLAGS='$(libgltf_CPPFLAGS)') \
		&& $(MAKE) \
	)

endif

# vim: set noet sw=4 ts=4:
