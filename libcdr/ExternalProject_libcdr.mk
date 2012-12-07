# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libcdr))

$(eval $(call gb_ExternalProject_use_unpacked,libcdr,cdr))

$(eval $(call gb_ExternalProject_register_targets,libcdr,\
	build \
))

$(eval $(call gb_ExternalProject_use_externals,libcdr,\
	lcms2 \
	wpd \
	wpg \
))

ifeq ($(OS)$(COM),WNTMSC)

ifeq ($(VCVER),90)
$(call gb_ExternalProject_get_state_target,libcdr,build) :
	cd $(EXTERNAL_WORKDIR)/build/win32 \
	&& export LIBWPD_INCLUDE_DIR=$(OUTDIR)/inc/external \
	&& export LIBWPG_INCLUDE_DIR=$(OUTDIR)/inc/external \
	&& export LCMS2_INCLUDE_DIR=$(call gb_UnpackedTarball_get_dir,lcms2/include) \
	&& export ZLIB_INCLUDE_DIR=$(OUTDIR)/inc/external/zlib \
	&& $(COMPATH)/vcpackages/vcbuild.exe libcdr.vcproj "Release|Win32" \
	&& touch $@
else ifeq ($(VCVER),100)
$(call gb_ExternalProject_get_state_target,libcdr,build) :
	cd $(EXTERNAL_WORKDIR)/build/win32 \
	&& export LIBWPD_INCLUDE_DIR=$(OUTDIR)/inc/external \
	&& export LIBWPG_INCLUDE_DIR=$(OUTDIR)/inc/external \
	&& export LCMS2_INCLUDE_DIR=$(call gb_UnpackedTarball_get_dir,lcms2/include) \
	&& export ZLIB_INCLUDE_DIR=$(OUTDIR)/inc/external/zlib \
	&& '/cygdrive/c/Windows/Microsoft.NET/Framework/v4.0.30319/msbuild.exe' libcdr.vcxproj /p:Configuration=Release \
	&& touch $@
else
$(call gb_ExternalProject_get_state_target,libcdr,build) :
	cd $(EXTERNAL_WORKDIR)/build/win32 \
	&& export LIBWPD_INCLUDE_DIR=$(OUTDIR)/inc/external \
	&& export LIBWPG_INCLUDE_DIR=$(OUTDIR)/inc/external \
	&& export LCMS2_INCLUDE_DIR=$(call gb_UnpackedTarball_get_dir,lcms2/include) \
	&& export ZLIB_INCLUDE_DIR=$(OUTDIR)/inc/external/zlib \
	&& msbuild.exe libcdr.vcxproj /p:PlatformToolset=v110 /p:Configuration=Release \
	&& touch $@
endif

else

$(call gb_ExternalProject_get_state_target,libcdr,build) :
	cd $(EXTERNAL_WORKDIR) \
	&& PKG_CONFIG="" \
	./configure \
		--with-pic \
		--enable-static \
		--disable-shared \
		--without-docs \
		--disable-debug \
		--disable-werror \
		$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
	&& (cd $(EXTERNAL_WORKDIR)/src/lib && $(MAKE)) \
	&& touch $@

endif

# vim: set noet sw=4 ts=4:
