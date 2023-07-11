# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libcdr))

$(eval $(call gb_ExternalProject_use_autoconf,libcdr,build))

$(eval $(call gb_ExternalProject_register_targets,libcdr,\
	build \
))

$(eval $(call gb_ExternalProject_use_externals,libcdr,\
	boost_headers \
	icu \
	lcms2 \
	revenge \
	zlib \
))

$(call gb_ExternalProject_get_state_target,libcdr,build) :
	$(call gb_Trace_StartRange,libcdr,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		export PKG_CONFIG="" \
		&& unset MSYS_NO_PATHCONV && MAKE=$(MAKE) $(gb_RUN_CONFIGURE) ./configure \
			--with-pic \
			--enable-static \
			--disable-shared \
			--without-docs \
			--disable-tools \
			$(if $(ENABLE_DEBUG),--enable-debug,--disable-debug) \
			--disable-werror \
			--disable-weffc \
			$(if $(gb_FULLDEPS),,--disable-dependency-tracking) \
			$(if $(verbose),--disable-silent-rules,--enable-silent-rules) \
			CXXFLAGS="$(gb_CXXFLAGS) $(call gb_ExternalProject_get_build_flags,libcdr)" \
			CPPFLAGS="$(CPPFLAGS) $(BOOST_CPPFLAGS)" \
			LDFLAGS="$(call gb_ExternalProject_get_link_flags,libcdr)" \
			$(gb_CONFIGURE_PLATFORMS) \
		&& $(MAKE) \
	)
	$(call gb_Trace_EndRange,libcdr,EXTERNAL)

# vim: set noet sw=4 ts=4:
