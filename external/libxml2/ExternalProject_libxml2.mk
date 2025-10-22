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

ifeq ($(OS),WNT)
$(call gb_ExternalProject_use_external_project,libxml2,icu)

# remove VERSION file after the compilation, otherwise, libabw, libvisio,
# and probably other libraries fail to compile with
# workdir\UnpackedTarball\libxml2\version(1): error C2059: syntax error: 'constant
$(call gb_ExternalProject_get_state_target,libxml2,build):
	$(call gb_ExternalProject_run,build,\
		$(CMAKE) . \
			$(if $(filter 150,$(VCVER)),-G "Visual Studio 16 2019" -T v141) \
			$(if $(filter 160,$(VCVER)),-G "Visual Studio 16 2019") \
			-A $(gb_MSBUILD_PLATFORM) \
			-DLIBXML2_WITH_TESTS=OFF \
			-DLIBXML2_WITH_ICONV=OFF \
			-DLIBXML2_WITH_PYTHON=OFF \
			-DLIBXML2_WITH_SAX1=ON \
			-DLIBXML2_WITH_ICU=ON \
			-DICU_INCLUDE_DIR=$(call gb_UnpackedTarball_get_dir,icu)/source/common/ \
			-DICU_LIBRARY=$(call gb_UnpackedTarball_get_dir,icu)/source/lib/ \
			$(if $(MSVC_USE_DEBUG_RUNTIME), \
				-DICU_UC_LIBRARY_DEBUG=$(call gb_UnpackedTarball_get_dir,icu)/source/lib/icuucd.lib, \
				-DICU_UC_LIBRARY_RELEASE=$(call gb_UnpackedTarball_get_dir,icu)/source/lib/icuuc.lib) \
		&& $(CMAKE) --build . --config $(if $(MSVC_USE_DEBUG_RUNTIME),Debug,Release) \
		&& rm VERSION \
	)
else # OS!=WNT
$(call gb_ExternalProject_get_state_target,libxml2,build):
	$(call gb_ExternalProject_run,build,\
		./configure --disable-ipv6 --without-iconv --without-python --without-zlib --with-sax1 \
			--without-lzma \
			$(if $(debug),--with-run-debug) \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________URELIB) \
			LDFLAGS="$(if $(SYSBASE),-L$(SYSBASE)/usr/lib)" \
			CFLAGS="$(if $(SYSBASE),-I$(SYSBASE)/usr/include) $(if $(debug),-g)" \
			$(if $(filter TRUE,$(DISABLE_DYNLOADING)),--disable-shared,--disable-static) \
		&& $(MAKE) libxml2.la xmllint \
	)
endif

# vim: set noet sw=4 ts=4:
