# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libmspub))

$(eval $(call gb_ExternalProject_use_unpacked,libmspub,mspub))

$(eval $(call gb_ExternalProject_register_targets,libmspub,\
	build \
))

$(eval $(call gb_ExternalProject_use_externals,libmspub,\
	boost_headers \
	icu \
	wpd \
	wpg \
))

ifeq ($(OS)$(COM),WNTMSC)

ifeq ($(VCVER),90)
$(call gb_ExternalProject_get_state_target,libmspub,build) :
	cd $(EXTERNAL_WORKDIR)/build/win32 \
	&& export BOOST_INCLUDE_DIR=$(call gb_UnpackedTarball_get_dir,boost) \
	&& export LIBWPD_INCLUDE_DIR=$(OUTDIR)/inc/external \
	&& export LIBWPG_INCLUDE_DIR=$(OUTDIR)/inc/external \
	&& export ZLIB_INCLUDE_DIR=$(OUTDIR)/inc/external/zlib \
	&& export ICU_INCLUDE_DIR=$(OUTDIR)/inc/external \
	&& $(COMPATH)/vcpackages/vcbuild.exe libmspub.vcproj "Release|Win32" \
	&& touch $@
else ifeq ($(VCVER),100)
$(call gb_ExternalProject_get_state_target,libmspub,build) :
	cd $(EXTERNAL_WORKDIR)/build/win32 \
	&& export BOOST_INCLUDE_DIR=$(call gb_UnpackedTarball_get_dir,boost) \
	&& export LIBWPD_INCLUDE_DIR=$(OUTDIR)/inc/external \
	&& export LIBWPG_INCLUDE_DIR=$(OUTDIR)/inc/external \
	&& export ZLIB_INCLUDE_DIR=$(OUTDIR)/inc/external/zlib \
	&& export ICU_INCLUDE_DIR=$(OUTDIR)/inc/external \
	&& msbuild.exe libmspub.vcxproj /p:Configuration=Release \
	&& touch $@
else
$(call gb_ExternalProject_get_state_target,libmspub,build) :
	cd $(EXTERNAL_WORKDIR)/build/win32 \
	&& export BOOST_INCLUDE_DIR=$(call gb_UnpackedTarball_get_dir,boost) \
	&& export LIBWPD_INCLUDE_DIR=$(OUTDIR)/inc/external \
	&& export LIBWPG_INCLUDE_DIR=$(OUTDIR)/inc/external \
	&& export ZLIB_INCLUDE_DIR=$(OUTDIR)/inc/external/zlib \
	&& export ICU_INCLUDE_DIR=$(OUTDIR)/inc/external \
	&& msbuild.exe libmspub.vcxproj /p:PlatformToolset=v110 /p:VisualStudioVersion=11.0 /p:Configuration=Release \
	&& touch $@
endif

else

$(call gb_ExternalProject_get_state_target,libmspub,build) :
	cd $(EXTERNAL_WORKDIR) \
	&& export PKG_CONFIG="" \
	&& export ICU_LIBS=" " && export ICU_CFLAGS="-I$(OUTDIR)/inc/external" \
	&& export LIBMSPUB_CFLAGS="$(WPG_CFLAGS) $(WPD_CFLAGS)" \
	&& export LIBMSPUB_LIBS="$(WPG_LIBS) $(WPD_LIBS)" \
	&& ./configure \
		--with-pic \
		--enable-static \
		--disable-shared \
		--without-docs \
		--disable-debug \
		--disable-werror \
		--disable-weffc \
		$(if $(filter NO,$(SYSTEM_BOOST)),CXXFLAGS=-I$(call gb_UnpackedTarball_get_dir,boost),CXXFLAGS=$(BOOST_CPPFLAGS)) \
		$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
	&& (cd $(EXTERNAL_WORKDIR)/src/lib && $(MAKE)) \
	&& touch $@

endif

# vim: set noet sw=4 ts=4:
