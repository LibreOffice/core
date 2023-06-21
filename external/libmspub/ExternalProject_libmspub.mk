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

libmspub_CPPFLAGS := $(CPPFLAGS) $(BOOST_CPPFLAGS)
# Needed when workdir/UnpackedTarball/libmspub/src/lib/MSPUBCollector.cpp includes Boost 1.79.0
# boost/multi_array.hpp, which indirectly includes
# workdir/UnpackedTarball/boost/boost/functional.hpp using std::unary_/binary_function:
ifeq ($(HAVE_LIBCPP),TRUE)
libmspub_CPPFLAGS += -D_LIBCPP_ENABLE_CXX17_REMOVED_UNARY_BINARY_FUNCTION
endif

$(call gb_ExternalProject_get_state_target,libmspub,build) :
	$(call gb_Trace_StartRange,libmspub,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		export PKG_CONFIG="" \
		&& MAKE=$(MAKE) $(gb_RUN_CONFIGURE) ./configure \
			--with-pic \
			--enable-static \
			--disable-shared \
			--without-docs \
			--disable-tools \
			$(if $(ENABLE_DEBUG),--enable-debug,--disable-debug) \
			--disable-werror \
			--disable-weffc \
			$(if $(verbose),--disable-silent-rules,--enable-silent-rules) \
			CXXFLAGS="$(gb_CXXFLAGS) $(call gb_ExternalProject_get_build_flags,libmspub)" \
			CPPFLAGS="$(libmspub_CPPFLAGS)" \
			LDFLAGS="$(call gb_ExternalProject_get_link_flags,libmspub)" \
			$(gb_CONFIGURE_PLATFORMS) \
		&& $(MAKE) \
	)
	$(call gb_Trace_EndRange,libmspub,EXTERNAL)

# vim: set noet sw=4 ts=4:
