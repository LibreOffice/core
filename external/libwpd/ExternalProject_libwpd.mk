# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libwpd))

$(eval $(call gb_ExternalProject_use_autoconf,libwpd,build))

$(eval $(call gb_ExternalProject_register_targets,libwpd,\
	build \
))

$(eval $(call gb_ExternalProject_use_externals,libwpd,\
	revenge \
))

$(call gb_ExternalProject_get_state_target,libwpd,build) :
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
			$(if $(filter MACOSX,$(OS)), \
				--disable-werror \
				--prefix=/@.__________________________________________________OOO) \
			$(if $(verbose),--disable-silent-rules,--enable-silent-rules) \
			$(if $(filter TRUE,$(DISABLE_DYNLOADING)),CFLAGS="$(CFLAGS) $(gb_VISIBILITY_FLAGS) $(gb_COMPILEROPTFLAGS)" CXXFLAGS="$(CXXFLAGS) $(gb_VISIBILITY_FLAGS) $(gb_VISIBILITY_FLAGS_CXX) $(gb_COMPILEROPTFLAGS)") \
			$(if $(filter LINUX,$(OS)),$(if $(SYSTEM_REVENGE),, \
				'LDFLAGS=-Wl$(COMMA)-z$(COMMA)origin \
					-Wl$(COMMA)-rpath$(COMMA)\$$$$ORIGIN')) \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
		&& $(MAKE) \
		$(if $(filter MACOSX,$(OS)),\
			&& $(PERL) $(SRCDIR)/solenv/bin/macosx-change-install-names.pl shl OOO \
				$(gb_Package_SOURCEDIR_libwpd)/src/lib/.libs/libwpd-0.10.10.dylib \
		) \
	)

# vim: set noet sw=4 ts=4:
