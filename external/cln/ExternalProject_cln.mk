# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,cln))

$(eval $(call gb_ExternalProject_register_targets,cln,\
        build \
))

cln_CPPCLAGS=$(CPPFLAGS)

# TODO: Enable usage of libgmp
# Note: --prefix is also written into pkg-config file cln.pc, which is then used by GiNaC
# Note: The setting of CFLAGS and CPPFLAGS is ignored by the cln configure script
$(call gb_ExternalProject_get_state_target,cln,build):
	$(call gb_Trace_StartRange,cln,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		$(gb_RUN_CONFIGURE) ./configure --enable-shared --with-pic --without-gmp \
			--prefix=$(call gb_UnpackedTarball_get_dir,cln)/install --includedir=$(call gb_UnpackedTarball_get_dir,cln)/include \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(filter AIX,$(OS)),CFLAGS="-D_LINUX_SOURCE_COMPAT") \
			$(if $(cln_CPPFLAGS),CPPFLAGS='$(cln_CPPFLAGS)') \
			CXXFLAGS="$(CXXFLAGS) $(gb_EMSCRIPTEN_CPPFLAGS) $(if $(ENABLE_OPTIMIZED),$(gb_COMPILEROPTFLAGS),$(gb_COMPILERNOOPTFLAGS)) $(if $(debug),$(gb_DEBUGINFO_FLAGS))" \
		&& cd src && $(MAKE) install \
	)
	$(call gb_Trace_EndRange,cln,EXTERNAL)

# vim: set noet sw=4 ts=4:
