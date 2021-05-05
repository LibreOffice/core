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
	$(call gb_Trace_StartRange,harfbuzz,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		$(if $(CROSS_COMPILING),ICU_CONFIG=$(SRCDIR)/external/icu/cross-bin/icu-config) \
		$(if $(SYSTEM_ICU),,ICU_CONFIG=$(SRCDIR)/external/icu/cross-bin/icu-config) \
		GRAPHITE2_CFLAGS="$(GRAPHITE_CFLAGS)" \
		GRAPHITE2_LIBS="$(GRAPHITE_LIBS)" \
		$(gb_RUN_CONFIGURE) ./configure \
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
			$(if $(verbose),--disable-silent-rules,--enable-silent-rules) \
			$(if $(gb_FULLDEPS),,--disable-dependency-tracking) \
			--libdir=$(call gb_UnpackedTarball_get_dir,harfbuzz/src/.libs) \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			CXXFLAGS=' \
				$(if $(filter ANDROID,$(OS)),-DHB_NO_MMAP=1,) \
				$(if $(call gb_Module__symbols_enabled,harfbuzz),$(gb_DEBUGINFO_FLAGS)) \
				$(if $(ENABLE_OPTIMIZED), \
					$(gb_COMPILEROPTFLAGS),$(gb_COMPILERNOOPTFLAGS)) \
				$(if $(ENABLE_RUNTIME_OPTIMIZATIONS),,-frtti) \
				$(CXXFLAGS) $(CXXFLAGS_CXX11) \
				$(ICU_UCHAR_TYPE) \
				$(if $(filter LINUX,$(OS)),-fvisibility=hidden)' \
			MAKE=$(MAKE) \
		&& (cd $(EXTERNAL_WORKDIR)/src && $(MAKE) lib) \
	)
	$(call gb_Trace_EndRange,harfbuzz,EXTERNAL)

# vim: set noet sw=4 ts=4:
