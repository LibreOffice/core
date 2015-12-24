# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libvisio))

$(eval $(call gb_ExternalProject_use_autoconf,libvisio,build))

$(eval $(call gb_ExternalProject_register_targets,libvisio,\
	build \
))

$(eval $(call gb_ExternalProject_use_externals,libvisio,\
	boost_headers \
	icu \
	libxml2 \
	revenge \
	zlib \
))

$(call gb_ExternalProject_get_state_target,libvisio,build) :
	$(call gb_ExternalProject_run,build,\
		export PKG_CONFIG="" \
		&& MAKE=$(MAKE) ./configure \
			--with-pic \
			--enable-static \
			--disable-shared \
			--without-docs \
			--disable-tools \
			$(if $(ENABLE_DEBUG),--enable-debug,--disable-debug) \
			--disable-werror \
			$(if $(verbose),--disable-silent-rules,--enable-silent-rules) \
			CXXFLAGS="$(CXXFLAGS) $(if $(SYSTEM_BOOST),$(BOOST_CPPFLAGS),-I$(call gb_UnpackedTarball_get_dir,boost))" \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
		&& $(MAKE) \
	)

# vim: set noet sw=4 ts=4:
