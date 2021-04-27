# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,librevenge))

$(eval $(call gb_ExternalProject_use_autoconf,librevenge,build))

$(eval $(call gb_ExternalProject_register_targets,librevenge,\
	build \
))

$(eval $(call gb_ExternalProject_use_externals,librevenge,\
	boost_headers \
))

$(call gb_ExternalProject_get_state_target,librevenge,build) :
	$(call gb_Trace_StartRange,librevenge,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		export PKG_CONFIG="" \
		&& $(gb_RUN_CONFIGURE) ./configure \
			--with-pic \
			$(if $(DISABLE_DYNLOADING), \
				--disable-shared --enable-static, \
				--enable-shared --disable-static) \
			$(if $(ENABLE_DEBUG),--enable-debug,--disable-debug) \
			--disable-werror \
			--disable-weffc \
			--disable-streams \
			--disable-generators \
			--without-docs \
			$(if $(verbose),--disable-silent-rules,--enable-silent-rules) \
			CXXFLAGS="$(gb_CXXFLAGS) $(if $(ENABLE_OPTIMIZED),$(gb_COMPILEROPTFLAGS),$(gb_COMPILERNOOPTFLAGS))" \
			CPPFLAGS="$(CPPFLAGS) $(BOOST_CPPFLAGS)" \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
		&& $(MAKE) \
		$(if $(filter MACOSX,$(OS)),\
			&& $(PERL) $(SRCDIR)/solenv/bin/macosx-change-install-names.pl shl OOO \
				$(EXTERNAL_WORKDIR)/src/lib/.libs/librevenge-0.0.0.dylib \
		) \
	)
	$(call gb_Trace_EndRange,librevenge,EXTERNAL)

# vim: set noet sw=4 ts=4:
