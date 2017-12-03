# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,gpgmepp))

$(eval $(call gb_ExternalProject_register_targets,gpgmepp,\
	build \
))

$(eval $(call gb_ExternalProject_use_autoconf,gpgmepp,build))

$(eval $(call gb_ExternalProject_use_externals,gpgmepp,\
       libgpg-error \
       libassuan \
))

ifeq ($(COM),MSC)
gb_ExternalProject_gpgmepp_host := $(if $(filter INTEL,$(CPUNAME)),i686-mingw32,x86_64-w64-mingw32)
gb_ExternalProject_gpgmepp_target := $(if $(filter INTEL,$(CPUNAME)),pe-i386,pe-x86-64)
$(call gb_ExternalProject_get_state_target,gpgmepp,build): $(call gb_Executable_get_target,cpp)
	$(call gb_ExternalProject_run,build,\
		autoreconf \
		&& ./configure \
		   --disable-shared \
		   --disable-languages \
		   --disable-gpgconf-test \
		   --disable-gpg-test \
		   --disable-gpgsm-test \
		   --disable-g13-test \
		   --disable-glibtest \
		   CFLAGS='$(CFLAGS) \
				$(if $(ENABLE_OPTIMIZED), \
					$(gb_COMPILEROPTFLAGS),$(gb_COMPILERNOOPTFLAGS)) \
				$(if $(ENABLE_DEBUG),$(gb_DEBUG_CFLAGS)) \
				$(if $(filter $(true),$(gb_SYMBOL)),$(gb_DEBUGINFO_FLAGS))' \
		   --host=$(gb_ExternalProject_libgpg-error_host) \
		   RC='windres -O COFF --target=$(gb_ExternalProject_libgpg-error_target) --preprocessor='\''$(call gb_Executable_get_target,cpp) -+ -DRC_INVOKED -DWINAPI_FAMILY=0 $(SOLARINC)'\' \
		   MAKE=$(MAKE) \
	    && $(MAKE) \
	)
else
$(call gb_ExternalProject_get_state_target,gpgmepp,build):
	$(call gb_ExternalProject_run,build,\
		autoreconf \
		&& ./configure \
		   --enable-languages="cpp" \
		   GPG_ERROR_CFLAGS="$(GPG_ERROR_CFLAGS)" \
		   GPG_ERROR_LIBS="$(GPG_ERROR_LIBS)" \
		   LIBASSUAN_CFLAGS="$(LIBASSUAN_CFLAGS)" \
		   LIBASSUAN_LIBS="$(LIBASSUAN_LIBS)" \
		   CFLAGS='$(CFLAGS) \
				$(if $(ENABLE_OPTIMIZED), \
					$(gb_COMPILEROPTFLAGS),$(gb_COMPILERNOOPTFLAGS)) \
				$(if $(ENABLE_DEBUG),$(gb_DEBUG_CFLAGS)) \
				$(if $(filter $(true),$(gb_SYMBOL)),$(gb_DEBUGINFO_FLAGS))' \
		   $(if $(filter LINUX,$(OS)), \
				'LDFLAGS=-Wl$(COMMA)-z$(COMMA)origin \
					-Wl$(COMMA)-rpath$(COMMA)\$$$$ORIGIN') \
		   $(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
	  && $(MAKE) \
	)
endif

# vim: set noet sw=4 ts=4:
