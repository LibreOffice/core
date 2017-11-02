# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libwpg))

$(eval $(call gb_ExternalProject_use_autoconf,libwpg,build))

$(eval $(call gb_ExternalProject_register_targets,libwpg,\
	build \
))

$(eval $(call gb_ExternalProject_use_externals,libwpg,\
	revenge \
	wpd \
))

$(call gb_ExternalProject_get_state_target,libwpg,build) :
	$(call gb_ExternalProject_run,build,\
		export PKG_CONFIG="" \
		&& MAKE=$(MAKE) ./configure \
			--with-pic \
			$(if $(DISABLE_DYNLOADING), \
				--disable-shared --enable-static, \
				--enable-shared --disable-static) \
			--without-docs \
			--disable-tools \
			--disable-debug \
			--disable-werror \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
			$(if $(verbose),--disable-silent-rules,--enable-silent-rules) \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			CXXFLAGS="$(gb_CXXFLAGS) $(if $(ENABLE_OPTIMIZED),$(gb_COMPILEROPTFLAGS),$(gb_COMPILERNOOPTFLAGS))" \
			$(if $(filter LINUX,$(OS)), \
				'LDFLAGS=$(LDFLAGS) -Wl$(COMMA)-z$(COMMA)origin \
					-Wl$(COMMA)-rpath$(COMMA)\$$$$ORIGIN') \
		&& $(MAKE) \
		$(if $(filter MACOSX,$(OS)),\
			&& $(PERL) $(SRCDIR)/solenv/bin/macosx-change-install-names.pl shl OOO \
				$(EXTERNAL_WORKDIR)/src/lib/.libs/libwpg-0.3.3.dylib \
		) \
	)

# vim: set noet sw=4 ts=4:
