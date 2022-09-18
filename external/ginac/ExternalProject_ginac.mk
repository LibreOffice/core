# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,ginac))

$(eval $(call gb_ExternalProject_use_autoconf,ginac,configure))
$(eval $(call gb_ExternalProject_use_autoconf,ginac,build))

$(eval $(call gb_ExternalProject_register_targets,ginac,\
	configure \
	build \
))

ginac_CPPCLAGS=$(CPPFLAGS)

# Note: Make install is required to get a clean include file directory
$(call gb_ExternalProject_get_state_target,ginac,build): $(call gb_ExternalProject_get_state_target,ginac,configure)
	$(call gb_Trace_StartRange,ginac,EXTERNAL)
	+$(call gb_ExternalProject_run,build,\
	cd ginac && $(MAKE) install \
	)
	$(call gb_Trace_EndRange,ginac,EXTERNAL)

# Note: The setting of CPPFLAGS and CXXFLAGS is ignored by the ginac configure script?
$(call gb_ExternalProject_get_state_target,ginac,configure):
	$(call gb_Trace_StartRange,ginac,EXTERNAL)
ifeq ($(COM),MSC)
	$(call gb_ExternalProject_run,configure,\
		MAKE=$(MAKE) $(gb_RUN_CONFIGURE) ./configure \
			--build=$(BUILD_PLATFORM) \
			--host=$(HOST_PLATFORM) \
			--disable-shared --enable-static \
			PKG_CONFIG_PATH=$(call gb_UnpackedTarball_get_dir,cln) \
			--prefix=$(call gb_UnpackedTarball_get_dir,ginac)/instdir \
			$(if $(ginac_CPPFLAGS),CPPFLAGS='$(ginac_CPPFLAGS)') \
			CPPFLAGS="$(CPPFLAGS) -MD -EHsc -Zc:__cplusplus" \
			CXXFLAGS="$(CXXFLAGS) $(gb_EMSCRIPTEN_CPPFLAGS) $(if $(ENABLE_OPTIMIZED),$(gb_COMPILEROPTFLAGS),$(gb_COMPILERNOOPTFLAGS)) $(if $(debug),$(gb_DEBUGINFO_FLAGS))" \
	)
else
	$(call gb_ExternalProject_run,configure,\
		$(gb_RUN_CONFIGURE) ./configure --enable-shared --with-pic \
			PKG_CONFIG_PATH=$(call gb_UnpackedTarball_get_dir,cln) \
			--prefix=$(call gb_UnpackedTarball_get_dir,ginac)/instdir \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM))\
			$(if $(filter AIX,$(OS)),CFLAGS="-D_LINUX_SOURCE_COMPAT") \
			$(if $(ginac_CPPFLAGS),CPPFLAGS='$(ginac_CPPFLAGS)') \
			CXXFLAGS="$(CXXFLAGS) $(gb_EMSCRIPTEN_CPPFLAGS) $(if $(ENABLE_OPTIMIZED),$(gb_COMPILEROPTFLAGS),$(gb_COMPILERNOOPTFLAGS)) $(if $(debug),$(gb_DEBUGINFO_FLAGS))" \
	)
endif
	$(call gb_Trace_EndRange,ginac,EXTERNAL)

# vim: set noet sw=4 ts=4:
