# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,glyphy))

$(eval $(call gb_ExternalProject_use_autoconf,glyphy,build))

$(eval $(call gb_ExternalProject_register_targets,glyphy,\
	build \
))

$(eval $(call gb_ExternalProject_use_externals,glyphy, \
    glew \
))

$(call gb_ExternalProject_get_state_target,glyphy,build) :
	$(call gb_ExternalProject_run,build,\
		MAKE=$(MAKE) ./configure \
			--with-pic \
			$(if $(DISABLE_DYNLOADING), \
				--enable-static --disable-shared \
			, \
				--enable-shared --disable-static \
			) \
			$(if $(ENABLE_DEBUG),--enable-debug,--disable-debug) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
		&& $(if $(verbose),V=1) \
		   $(MAKE) \
	)

# vim: set noet sw=4 ts=4:
