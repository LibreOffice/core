# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libetonyek))

$(eval $(call gb_ExternalProject_use_autoconf,libetonyek,build))

$(eval $(call gb_ExternalProject_register_targets,libetonyek,\
	build \
))

$(eval $(call gb_ExternalProject_use_externals,libetonyek,\
	boost_headers \
	glm \
	libxml2 \
	revenge \
	zlib \
))

$(call gb_ExternalProject_get_state_target,libetonyek,build) :
	$(call gb_ExternalProject_run,build,\
		export PKG_CONFIG="" \
		&& MAKE=$(MAKE) ./configure \
			--with-pic \
			--enable-shared \
			--disable-static \
			--without-docs \
			$(if $(ENABLE_DEBUG),--enable-debug,--disable-debug) \
			--disable-werror \
			--disable-weffc \
			--without-tools \
			$(if $(VERBOSE)$(verbose),--disable-silent-rules,--enable-silent-rules) \
			BOOST_CFLAGS="$(if $(SYSTEM_BOOST),$(BOOST_CPPFLAGS),-I$(call gb_UnpackedTarball_get_dir,boost))" \
			XML_CFLAGS="$(LIBXML_CFLAGS)" \
			XML_LIBS="$(LIBXML_LIBS)" \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
		&& $(MAKE) \
	)

# vim: set noet sw=4 ts=4:
