# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libodfgen))

$(eval $(call gb_ExternalProject_use_autoconf,libodfgen,build))

$(eval $(call gb_ExternalProject_register_targets,libodfgen,\
	build \
))

$(eval $(call gb_ExternalProject_use_externals,libodfgen,\
	boost_headers \
	revenge \
))

$(call gb_ExternalProject_get_state_target,libodfgen,build) :
	$(call gb_ExternalProject_run,build,\
		export PKG_CONFIG="" \
		&& ./configure \
			--with-pic \
			$(if $(DISABLE_DYNLOADING), \
				--enable-static --disable-shared \
			, \
				--enable-shared --disable-static \
			) \
			--disable-debug \
			--disable-werror \
			--disable-weffc \
			--without-docs \
			$(if $(VERBOSE)$(verbose),--disable-silent-rules,--enable-silent-rules) \
			CXXFLAGS="$(if $(SYSTEM_BOOST),$(BOOST_CPPFLAGS),-I$(call gb_UnpackedTarball_get_dir,boost) -I$(BUILDDIR)/config_$(gb_Side))" \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
		&& $(MAKE) \
		$(if $(filter MACOSX,$(OS)),\
			&& $(PERL) $(SRCDIR)/solenv/bin/macosx-change-install-names.pl shl OOO \
				$(EXTERNAL_WORKDIR)/src/.libs/libodfgen-0.1.1.dylib \
		) \
	)

# vim: set noet sw=4 ts=4:
