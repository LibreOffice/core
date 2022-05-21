# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libtiff))

$(eval $(call gb_ExternalProject_register_targets,libtiff,\
	build \
))

$(eval $(call gb_ExternalProject_use_autoconf,libtiff,build))

$(call gb_ExternalProject_get_state_target,libtiff,build) :
	$(call gb_Trace_StartRange,libtiff,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		export PKG_CONFIG="" \
		&& MAKE=$(MAKE) $(gb_RUN_CONFIGURE) ./configure \
			--enable-static \
			--with-pic \
			--disable-shared \
			--disable-cxx \
			--disable-libdeflate \
			--disable-jbig \
			--disable-jpeg \
			--disable-lerc \
			--disable-lzma \
			--disable-mdi \
			--disable-webp \
			--disable-win32-io \
			--disable-zstd \
			--without-x \
			$(if $(verbose),--disable-silent-rules,--enable-silent-rules) \
			CFLAGS="$(CFLAGS) $(call gb_ExternalProject_get_build_flags,libtiff)" \
			CPPFLAGS="$(CPPFLAGS) $(BOOST_CPPFLAGS)" \
			LDFLAGS="$(call gb_ExternalProject_get_link_flags,libtiff)" \
			$(gb_CONFIGURE_PLATFORMS) \
		&& $(MAKE) \
	)
	$(call gb_Trace_EndRange,libtiff,EXTERNAL)

# vim: set noet sw=4 ts=4:
