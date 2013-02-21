# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libvisio))

$(eval $(call gb_ExternalProject_use_unpacked,libvisio,visio))

$(eval $(call gb_ExternalProject_register_targets,libvisio,\
	build \
))

$(eval $(call gb_ExternalProject_use_externals,libvisio,\
	boost_headers \
	wpd \
	wpg \
	libxml2 \
))

ifeq ($(OS)$(COM),WNTMSC)

ifeq ($(VCVER),90)
$(call gb_ExternalProject_get_state_target,libvisio,build) :
	$(call gb_ExternalProject_run,build,\
		export BOOST_INCLUDE_DIR=$(call gb_UnpackedTarball_get_dir,boost) \
		&& export LIBWPD_INCLUDE_DIR=$(OUTDIR)/inc/external \
		&& export LIBWPG_INCLUDE_DIR=$(OUTDIR)/inc/external \
		&& export LIBXML_INCLUDE_DIR=$(call gb_UnpackedTarball_get_dir,xml2)/include \
		&& export ZLIB_INCLUDE_DIR=$(OUTDIR)/inc/external/zlib \
		&& $(COMPATH)/vcpackages/vcbuild.exe libvisio.vcproj "Release|Win32" \
	,build/win32)
else ifeq ($(VCVER),100)
$(call gb_ExternalProject_get_state_target,libvisio,build) :
	$(call gb_ExternalProject_run,build,\
		export BOOST_INCLUDE_DIR=$(call gb_UnpackedTarball_get_dir,boost) \
		&& export LIBWPD_INCLUDE_DIR=$(OUTDIR)/inc/external \
		&& export LIBWPG_INCLUDE_DIR=$(OUTDIR)/inc/external \
		&& export LIBXML_INCLUDE_DIR=$(call gb_UnpackedTarball_get_dir,xml2)/include \
		&& export ZLIB_INCLUDE_DIR=$(OUTDIR)/inc/external/zlib \
		&& msbuild.exe libvisio.vcxproj /p:Configuration=Release \
	,build/win32)
else
$(call gb_ExternalProject_get_state_target,libvisio,build) :
	$(call gb_ExternalProject_run,build,\
		export BOOST_INCLUDE_DIR=$(call gb_UnpackedTarball_get_dir,boost) \
		&& export LIBWPD_INCLUDE_DIR=$(OUTDIR)/inc/external \
		&& export LIBWPG_INCLUDE_DIR=$(OUTDIR)/inc/external \
		&& export LIBXML_INCLUDE_DIR=$(call gb_UnpackedTarball_get_dir,xml2)/include \
		&& export ZLIB_INCLUDE_DIR=$(OUTDIR)/inc/external/zlib \
		&& msbuild.exe libvisio.vcxproj /p:PlatformToolset=v110 /p:VisualStudioVersion=11.0 /p:Configuration=Release \
	,build/win32)
endif

else

$(call gb_ExternalProject_get_state_target,libvisio,build) :
	$(call gb_ExternalProject_run,build,\
		PKG_CONFIG="" \
		./configure \
			--with-pic \
			--enable-static \
			--disable-shared \
			--without-docs \
			--disable-debug \
			--disable-werror \
			CXXFLAGS="$(if $(filter NO,$(SYSTEM_BOOST)),-I$(call gb_UnpackedTarball_get_dir,boost),$(BOOST_CPPFLAGS)) \
			$(if $(filter NO,$(SYSTEM_LIBXML)),-I$(call gb_UnpackedTarball_get_dir,xml2)/include)" \
			$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
		&& (cd $(EXTERNAL_WORKDIR)/src/lib && $(MAKE)) \
	)

endif

# vim: set noet sw=4 ts=4:
