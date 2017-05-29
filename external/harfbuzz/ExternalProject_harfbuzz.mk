# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,harfbuzz))

$(eval $(call gb_ExternalProject_use_autoconf,harfbuzz,build))

$(eval $(call gb_ExternalProject_register_targets,harfbuzz,\
	build \
))

$(eval $(call gb_ExternalProject_use_externals,harfbuzz,\
	icu \
	graphite \
))

$(call gb_ExternalProject_get_state_target,harfbuzz,build) :
	$(call gb_ExternalProject_run,build,\
		$(if $(CROSS_COMPILING),ICU_CONFIG=$(SRCDIR)/external/icu/cross-bin/icu-config) \
		$(if $(SYSTEM_ICU),,ICU_CONFIG=$(SRCDIR)/external/icu/cross-bin/icu-config) \
		GRAPHITE2_CFLAGS="$(GRAPHITE_CFLAGS)" \
		GRAPHITE2_LIBS="$(GRAPHITE_LIBS)" \
		./configure \
			--enable-static \
			--disable-shared \
			--disable-gtk-doc \
			--with-pic \
			--with-icu=builtin \
			--with-freetype=no \
			--with-fontconfig=no \
			--with-cairo=no \
			--with-glib=no \
			--with-graphite2=yes \
			$(if $(filter IOS MACOSX,$(OS)),--with-coretext=yes) \
			$(if $(verbose),--disable-silent-rules,--enable-silent-rules) \
			--libdir=$(call gb_UnpackedTarball_get_dir,harfbuzz/src/.libs) \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			CXXFLAGS=' \
				$(if $(filter $(true),$(gb_SYMBOL)),$(gb_DEBUGINFO_FLAGS)) \
				$(if $(debug), \
					$(gb_COMPILERNOOPTFLAGS) $(gb_DEBUG_CFLAGS) \
						$(gb_DEBUG_CXXFLAGS), \
					$(gb_COMPILEROPTFLAGS)) \
				$(CXXFLAGS) $(if $(filter LINUX,$(OS)),-fvisibility=hidden)' \
		&& (cd $(EXTERNAL_WORKDIR)/src && $(MAKE) lib) \
	)

# vim: set noet sw=4 ts=4:
