# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libwpd))

$(eval $(call gb_ExternalProject_use_autoconf,libwpd,build))

$(eval $(call gb_ExternalProject_register_targets,libwpd,\
	build \
))

$(call gb_ExternalProject_get_state_target,libwpd,build) :
	$(call gb_ExternalProject_run,build,\
		$(if $(filter TRUE,$(DISABLE_DYNLOADING)),CFLAGS="$(CFLAGS) $(gb_VISIBILITY_FLAGS) $(gb_COMPILEROPTFLAGS)" CXXFLAGS="$(CXXFLAGS) $(gb_VISIBILITY_FLAGS) $(gb_COMPILEROPTFLAGS)") \
		./configure \
			--with-pic \
			--enable-static \
			--disable-shared \
			--without-stream \
			--without-docs \
			--disable-debug \
			$(if $(filter MACOSX,$(OS)),--disable-werror) \
			$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
		&& $(if $(VERBOSE)$(verbose),V=1) \
		   $(MAKE) \
	)

# vim: set noet sw=4 ts=4:
