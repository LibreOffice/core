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
gb_ExternalProject_libgpg-error_host := $(if $(filter INTEL,$(CPUNAME)),i686-mingw32,x86_64-w64-mingw32)
gb_ExternalProject_libgpg-error_target := $(if $(filter INTEL,$(CPUNAME)),pe-i386,pe-x86-64)
$(call gb_ExternalProject_get_state_target,libgpg-error,build): $(call gb_Executable_get_target,cpp)
	$(call gb_ExternalProject_run,build,\
		MAKE=$(MAKE) ./configure \
			--enable-static \
			--disable-shared \
			--disable-rpath \
			--disable-languages \
			--disable-doc \
			--disable-tests \
			--host=$(gb_ExternalProject_libgpg-error_host) \
			RC='windres -O COFF --target=$(gb_ExternalProject_libgpg-error_target) --preprocessor='\''$(call gb_Executable_get_target,cpp) -+ -DRC_INVOKED -DWINAPI_FAMILY=0 $(SOLARINC)'\' \
	    && $(MAKE) \
	)
else
$(call gb_ExternalProject_get_state_target,libgpg-error,build):
	$(call gb_ExternalProject_run,build,\
		MAKE=$(MAKE) ./configure \
			--disable-rpath \
			--disable-languages \
			--disable-doc \
			$(if $(filter LINUX,$(OS)), \
				'LDFLAGS=-Wl$(COMMA)-z$(COMMA)origin \
					-Wl$(COMMA)-rpath$(COMMA)\$$$$ORIGIN') \
			CPPFLAGS=" $(SOLARINC)" \
			$(if $(filter MSC,$(COM)),--force_use_syscfg=true) \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
	                $(if $(filter TRUE,$(DISABLE_DYNLOADING)),--disable-shared,--disable-static) \
	  && $(MAKE) \
	)

endif
# vim: set noet sw=4 ts=4:
