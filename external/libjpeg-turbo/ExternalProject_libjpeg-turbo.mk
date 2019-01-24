# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libjpeg-turbo))

$(eval $(call gb_ExternalProject_use_autoconf,libjpeg-turbo,configure))
$(eval $(call gb_ExternalProject_use_autoconf,libjpeg-turbo,build))

$(eval $(call gb_ExternalProject_register_targets,libjpeg-turbo,\
	configure \
	build \
))

$(call gb_ExternalProject_get_state_target,libjpeg-turbo,build) : $(call gb_ExternalProject_get_state_target,libjpeg-turbo,configure)
	+$(call gb_ExternalProject_run,build,\
		$(MAKE) \
	)

$(call gb_ExternalProject_get_state_target,libjpeg-turbo,configure) :
	$(call gb_ExternalProject_run,configure,\
		MAKE=$(MAKE) ./configure \
			--build=$(if $(filter WNT,$(OS)),$(if $(filter INTEL,$(CPUNAME)),i686-pc-cygwin,x86_64-pc-cygwin),$(BUILD_PLATFORM)) \
			--host=$(if $(filter WNT,$(OS)),$(if $(filter INTEL,$(CPUNAME)),i686-pc-cygwin,x86_64-pc-cygwin),$(HOST_PLATFORM)) \
			--with-pic \
			--enable-static \
			--disable-shared \
			--without-java \
			--without-turbojpeg \
			$(if $(NASM),,--without-simd) \
			CFLAGS='$(if $(debug),$(gb_DEBUGINFO_FLAGS)) $(if $(ENABLE_OPTIMIZED),$(gb_COMPILEROPTFLAGS),$(gb_COMPILERNOOPTFLAGS)) $(CFLAGS) $(gb_VISIBILITY_FLAGS)' \
	)

# vim: set noet sw=4 ts=4:
