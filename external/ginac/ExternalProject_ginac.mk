# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,ginac))

$(eval $(call gb_ExternalProject_register_targets,ginac,\
	build \
))

ginac_CPPCLAGS=$(CPPFLAGS)

# Note: The setting of CPPFLAGS and CXXFLAGS is ignored by the ginac configure script?
$(call gb_ExternalProject_get_state_target,ginac,build):
	$(call gb_Trace_StartRange,ginac,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		$(gb_RUN_CONFIGURE) ./configure --enable-shared --with-pic \
			PKG_CONFIG_PATH=$(call gb_UnpackedTarball_get_dir,cln) \
			--prefix=$(call gb_UnpackedTarball_get_dir,ginac)/install \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM))\
			$(if $(filter AIX,$(OS)),CFLAGS="-D_LINUX_SOURCE_COMPAT") \
			$(if $(ginac_CPPFLAGS),CPPFLAGS='$(ginac_CPPFLAGS)') \
			CXXFLAGS="$(CXXFLAGS) $(gb_EMSCRIPTEN_CPPFLAGS) $(if $(ENABLE_OPTIMIZED),$(gb_COMPILEROPTFLAGS),$(gb_COMPILERNOOPTFLAGS)) $(if $(debug),$(gb_DEBUGINFO_FLAGS))" \
		&& cd ginac && $(MAKE) install \
	)
	$(call gb_Trace_EndRange,ginac,EXTERNAL)

# vim: set noet sw=4 ts=4:
