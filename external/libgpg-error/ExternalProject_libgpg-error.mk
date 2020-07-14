# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libgpg-error))

$(eval $(call gb_ExternalProject_register_targets,libgpg-error,\
	build \
))

$(eval $(call gb_ExternalProject_use_autoconf,libgpg-error,build))

ifeq ($(COM),MSC)
$(call gb_ExternalProject_get_state_target,libgpg-error,build): $(call gb_Executable_get_target_for_build,cpp)
	$(call gb_Trace_StartRange,libgpg-error,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		$(gb_WIN_GPG_cross_setup_exports) \
		&& MAKE=$(MAKE) ./configure \
			--enable-static \
			--disable-shared \
			--disable-rpath \
			--disable-languages \
			--disable-doc \
			--disable-tests \
			$(gb_WIN_GPG_platform_switches) \
	    && $(MAKE) \
	)
	$(call gb_Trace_EndRange,libgpg-error,EXTERNAL)
else
$(call gb_ExternalProject_get_state_target,libgpg-error,build):
	$(call gb_Trace_StartRange,libgpg-error,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		MAKE=$(MAKE) ./configure \
			--disable-rpath \
			--disable-languages \
			--disable-doc \
			CPPFLAGS=" $(SOLARINC)" \
			$(if $(filter MSC,$(COM)),--force_use_syscfg=true) \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
	                $(if $(filter TRUE,$(DISABLE_DYNLOADING)),--disable-shared,--disable-static) \
	  && $(MAKE) \
	)
	$(call gb_Trace_EndRange,libgpg-error,EXTERNAL)

endif
# vim: set noet sw=4 ts=4:
