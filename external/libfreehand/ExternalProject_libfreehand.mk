# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libfreehand))

$(eval $(call gb_ExternalProject_use_autoconf,libfreehand,build))

$(eval $(call gb_ExternalProject_register_targets,libfreehand,\
	build \
))

$(eval $(call gb_ExternalProject_use_externals,libfreehand,\
	wpd \
	wpg \
	zlib \
))

$(call gb_ExternalProject_get_state_target,libfreehand,build) :
	$(call gb_ExternalProject_run,build,\
		export PKG_CONFIG="" \
		&& ./configure \
			--with-pic \
			--enable-static \
			--disable-shared \
			--without-docs \
			--disable-debug \
			--disable-werror \
			--disable-weffc \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
		&& (cd $(EXTERNAL_WORKDIR)/src/lib && \
		    $(if $(VERBOSE)$(verbose),V=1) \
		    $(MAKE)) \
	)

# vim: set noet sw=4 ts=4:
