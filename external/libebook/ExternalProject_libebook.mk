# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libebook))

$(eval $(call gb_ExternalProject_use_autoconf,libebook,build))

$(eval $(call gb_ExternalProject_register_targets,libebook,\
	build \
))

$(eval $(call gb_ExternalProject_use_externals,libebook,\
	boost_headers \
	icu \
	libxml2 \
	revenge \
	zlib \
))

# TODO: remove the generators/stream empty vars on libe-book update
$(call gb_ExternalProject_get_state_target,libebook,build) :
	$(call gb_ExternalProject_run,build,\
		export PKG_CONFIG="" \
		&& MAKE=$(MAKE) ./configure \
			--with-pic \
			--enable-static \
			--disable-shared \
			--without-docs \
			--without-tools \
			--disable-tests \
			$(if $(ENABLE_DEBUG),--enable-debug,--disable-debug) \
			$(if $(VERBOSE)$(verbose),--disable-silent-rules,--enable-silent-rules) \
			--disable-werror \
			--disable-weffc \
			CXXFLAGS="$(if $(SYSTEM_BOOST),$(BOOST_CPPFLAGS),-I$(call gb_UnpackedTarball_get_dir,boost)) \
				-DBOOST_ERROR_CODE_HEADER_ONLY -DBOOST_SYSTEM_NO_DEPRECATED" \
			XML_CFLAGS="$(LIBXML_CFLAGS)" \
			XML_LIBS="$(LIBXML_LIBS)" \
			REVENGE_GENERATORS_CFLAGS=' ' REVENGE_GENERATORS_LIBS=' ' REVENGE_STREAM_CFLAGS=' ' REVENGE_STREAM_LIBS=' ' \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
		&& $(MAKE) \
	)

# vim: set noet sw=4 ts=4:
