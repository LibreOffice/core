# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libxml2))

$(eval $(call gb_ExternalProject_register_targets,libxml2,\
	build \
))

ifeq ($(OS),EMSCRIPTEN)
$(call gb_ExternalProject_use_external_project,libxml2,icu)
endif

ifeq ($(OS),WNT)
$(call gb_ExternalProject_use_external_project,libxml2,icu)

# remove VERSION file after the compilation, otherwise, libabw, libvisio,
# and probably other libraries fail to compile with
# workdir\UnpackedTarball\libxml2\version(1): error C2059: syntax error: 'constant
$(call gb_ExternalProject_get_state_target,libxml2,build):
	$(call gb_Trace_StartRange,libxml2,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		$(CMAKE) . \
			$(if $(filter 17.%,$(VCVER)),-G "Visual Studio 17 2022") \
			$(if $(filter 18.%,$(VCVER)),-G "Visual Studio 18 2026") \
			-A $(gb_MSBUILD_PLATFORM) \
			-DLIBXML2_WITH_TESTS=OFF \
			-DLIBXML2_WITH_ICONV=OFF \
			-DLIBXML2_WITH_PYTHON=OFF \
			-DLIBXML2_WITH_SAX1=ON \
			-DLIBXML2_WITH_ICU=ON \
			-DICU_INCLUDE_DIR=$(gb_UnpackedTarball_workdir)/icu/source/common/ \
			-DICU_LIBRARY=$(gb_UnpackedTarball_workdir)/icu/source/lib/ \
			$(if $(MSVC_USE_DEBUG_RUNTIME), \
				-DICU_UC_LIBRARY_DEBUG=$(gb_UnpackedTarball_workdir)/icu/source/lib/icuucd.lib, \
				-DICU_UC_LIBRARY_RELEASE=$(gb_UnpackedTarball_workdir)/icu/source/lib/icuuc.lib) \
		&& $(CMAKE) --build . --config $(if $(MSVC_USE_DEBUG_RUNTIME),Debug,Release) \
		&& rm VERSION \
	)
	$(call gb_Trace_EndRange,libxml2,EXTERNAL)
else # OS!=WNT
$(call gb_ExternalProject_get_state_target,libxml2,build):
	$(call gb_Trace_StartRange,libxml2,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		$(gb_RUN_CONFIGURE) ./configure --without-iconv --without-python --without-zlib --with-sax1 \
			--without-lzma \
			$(if $(debug),--with-debug) \
			$(if $(verbose),--disable-silent-rules,--enable-silent-rules) \
			$(gb_CONFIGURE_PLATFORMS) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________URELIB) \
			LDFLAGS="$(LDFLAGS) $(call gb_ExternalProject_get_link_flags,libxml2) $(if $(SYSBASE),-L$(SYSBASE)/usr/lib)" \
			CFLAGS="$(CFLAGS) \
				$(if $(SYSBASE),-I$(SYSBASE)/usr/include) \
				$(call gb_ExternalProject_get_build_flags,libxml2)" \
			$(if $(filter TRUE,$(DISABLE_DYNLOADING)),--disable-shared,--disable-static) \
		&& $(MAKE) libxml2.la xmllint \
	)
	$(call gb_Trace_EndRange,libxml2,EXTERNAL)
endif

# vim: set noet sw=4 ts=4:
