# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libwps))

$(eval $(call gb_ExternalProject_use_autoconf,libwps,build))

$(eval $(call gb_ExternalProject_register_targets,libwps,\
	build \
))

$(eval $(call gb_ExternalProject_use_externals,libwps,\
	boost_headers \
	revenge \
))

$(call gb_ExternalProject_get_state_target,libwps,build) :
	$(call gb_ExternalProject_run,build,\
		export PKG_CONFIG="" \
		&& ./configure \
			--with-pic \
			$(if $(DISABLE_DYNLOADING), \
				--enable-static --disable-shared \
			, \
				--enable-shared --disable-static \
			) \
			--without-docs \
			--disable-tools \
			--disable-debug \
			--disable-werror \
			$(if $(VERBOSE)$(verbose),--disable-silent-rules,--enable-silent-rules) \
			CXXFLAGS="$(if $(SYSTEM_BOOST),$(BOOST_CPPFLAGS),-I$(call gb_UnpackedTarball_get_dir,boost) -I$(BUILDDIR)/config_$(gb_Side))" \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
		&& $(MAKE) \
		$(if $(filter MACOSX,$(OS)),\
			&& $(PERL) $(SRCDIR)/solenv/bin/macosx-change-install-names.pl shl OOO \
				$(EXTERNAL_WORKDIR)/src/lib/.libs/libwps-0.3.3.dylib \
		) \
	)

# vim: set noet sw=4 ts=4:
