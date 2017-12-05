# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libmspub))

$(eval $(call gb_ExternalProject_use_autoconf,libmspub,build))

$(eval $(call gb_ExternalProject_register_targets,libmspub,\
	build \
))

$(eval $(call gb_ExternalProject_use_externals,libmspub,\
	boost_headers \
	icu \
	revenge \
	zlib \
))

# -D_LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR is needed when building against libc++ in C++17 mode, as
# libmspub configure checking for boost/ptr_container/ptr_vector.hpp can (indirectly) include
# Boost 1.65.1 workdir/UnpackedTarball/boost/boost/ptr_container/detail/reversible_ptr_container.hpp
# which mentions std::auto_ptr:
$(call gb_ExternalProject_get_state_target,libmspub,build) :
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
			--disable-weffc \
			$(if $(verbose),--disable-silent-rules,--enable-silent-rules) \
			CXXFLAGS="$(gb_CXXFLAGS) $(if $(ENABLE_OPTIMIZED),$(gb_COMPILEROPTFLAGS),$(gb_COMPILERNOOPTFLAGS))" \
			CPPFLAGS="$(CPPFLAGS) $(ICU_UCHAR_TYPE) $(BOOST_CPPFLAGS) \
				-DBOOST_ERROR_CODE_HEADER_ONLY -DBOOST_SYSTEM_NO_DEPRECATED \
				-D_LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR" \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
		&& $(MAKE) \
	)

# vim: set noet sw=4 ts=4:
