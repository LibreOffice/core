# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libmwaw))

$(eval $(call gb_ExternalProject_use_unpacked,libmwaw,mwaw))

$(eval $(call gb_ExternalProject_register_targets,libmwaw,\
	build \
))

$(eval $(call gb_ExternalProject_use_externals,libmwaw,\
	boost_headers \
	wpd \
))

ifeq ($(OS)$(COM),WNTMSC)

ifeq ($(VCVER),90)
$(call gb_ExternalProject_get_state_target,libmwaw,build) :
	$(call gb_ExternalProject_run,build,\
		export BOOST_INCLUDE_DIR=$(call gb_UnpackedTarball_get_dir,boost) \
		&& export LIBWPD_INCLUDE_DIR=$(OUTDIR)/inc/external \
		&& export ZLIB_INCLUDE_DIR=$(OUTDIR)/inc/external/zlib \
		&& $(COMPATH)/vcpackages/vcbuild.exe libmwaw.vcproj "Release|Win32" \
	,build/win32)
else ifeq ($(VCVER),100)
$(call gb_ExternalProject_get_state_target,libmwaw,build) :
	$(call gb_ExternalProject_run,build,\
		export BOOST_INCLUDE_DIR=$(call gb_UnpackedTarball_get_dir,boost) \
		&& export LIBWPD_INCLUDE_DIR=$(OUTDIR)/inc/external \
		&& export ZLIB_INCLUDE_DIR=$(OUTDIR)/inc/external/zlib \
		&& msbuild.exe libmwaw.vcxproj /p:Configuration=Release \
	,build/win32)
else
$(call gb_ExternalProject_get_state_target,libmwaw,build) :
	$(call gb_ExternalProject_run,build,\
		export BOOST_INCLUDE_DIR=$(call gb_UnpackedTarball_get_dir,boost) \
		&& export LIBWPD_INCLUDE_DIR=$(OUTDIR)/inc/external \
		&& export ZLIB_INCLUDE_DIR=$(OUTDIR)/inc/external/zlib \
		&& msbuild.exe libmwaw.vcxproj /p:PlatformToolset=v110 /p:VisualStudioVersion=11.0 /p:Configuration=Release \
	,build/win32)
endif

else

$(call gb_ExternalProject_get_state_target,libmwaw,build) :
	$(call gb_ExternalProject_run,build,\
		export PKG_CONFIG="" \
		&& ./configure \
			--with-pic \
			--enable-static \
			--disable-shared \
			--without-docs \
			--disable-debug \
			--disable-werror \
			$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
		&& (cd $(EXTERNAL_WORKDIR)/src/lib && $(MAKE)) \
	)

endif

# vim: set noet sw=4 ts=4:
