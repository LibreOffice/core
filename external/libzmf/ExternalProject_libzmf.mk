# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libzmf))

$(eval $(call gb_ExternalProject_use_autoconf,libzmf,build))

$(eval $(call gb_ExternalProject_register_targets,libzmf,\
	build \
))

$(eval $(call gb_ExternalProject_use_externals,libzmf,\
	boost_headers \
	icu \
	png \
	revenge \
	zlib \
))

$(call gb_ExternalProject_get_state_target,libzmf,build) :
	$(call gb_ExternalProject_run,build,\
		export PKG_CONFIG="" \
		&& MAKE=$(MAKE) ./configure \
			--with-pic \
			--enable-static \
			--disable-shared \
			--without-docs \
			--disable-tools \
			--disable-debug \
			--disable-werror \
			--disable-weffc \
			$(if $(verbose),--disable-silent-rules,--enable-silent-rules) \
			CXXFLAGS="$(CXXFLAGS) $(CXXFLAGS_CXX11) $(if $(SYSTEM_BOOST),$(BOOST_CPPFLAGS),-I$(call gb_UnpackedTarball_get_dir,boost))" \
			REVENGE_GENERATORS_CFLAGS=' ' REVENGE_GENERATORS_LIBS=' ' REVENGE_STREAM_CFLAGS=' ' REVENGE_STREAM_LIBS=' ' \
			ax_cv_cxx_compile_cxx11=yes \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
		&& $(MAKE) -C src/lib \
	)

# vim: set noet sw=4 ts=4:
