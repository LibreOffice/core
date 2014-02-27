# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libwpg))

$(eval $(call gb_ExternalProject_use_autoconf,libwpg,build))

$(eval $(call gb_ExternalProject_register_targets,libwpg,\
	build \
))

$(eval $(call gb_ExternalProject_use_externals,libwpg,\
	wpd \
))

$(call gb_ExternalProject_get_state_target,libwpg,build) :
	$(call gb_ExternalProject_run,build,\
		export PKG_CONFIG="" \
		&& ./configure \
			--with-pic \
			--enable-static \
			--disable-shared \
			--without-docs \
			--disable-debug \
			--disable-werror \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
		&& (cd $(EXTERNAL_WORKDIR)/src/lib && \
		    $(if $(VERBOSE)$(verbose),V=1) \
		    $(MAKE)) \
	)

# vim: set noet sw=4 ts=4:
