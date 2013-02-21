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
	icu \
	lcms2 \
	wpd \
	wpg \
))

ifeq ($(OS)$(COM),WNTMSC)

ifeq ($(VCVER),90)
$(call gb_ExternalProject_get_state_target,libcdr,build) :
	$(call gb_ExternalProject_run,build,\
		export LIBWPD_INCLUDE_DIR=$(OUTDIR)/inc/external \
		&& export LIBWPG_INCLUDE_DIR=$(OUTDIR)/inc/external \
		&& export LCMS2_INCLUDE_DIR=$(call gb_UnpackedTarball_get_dir,lcms2/include) \
		&& export ZLIB_INCLUDE_DIR=$(OUTDIR)/inc/external/zlib \
		&& export ICU_INCLUDE_DIR=$(OUTDIR)/inc/external \
		&& $(COMPATH)/vcpackages/vcbuild.exe libcdr.vcproj "Release|Win32" \
	,build/win32)
else ifeq ($(VCVER),100)
$(call gb_ExternalProject_get_state_target,libcdr,build) :
	$(call gb_ExternalProject_run,build,\
		export LIBWPD_INCLUDE_DIR=$(OUTDIR)/inc/external \
		&& export LIBWPG_INCLUDE_DIR=$(OUTDIR)/inc/external \
		&& export LCMS2_INCLUDE_DIR=$(call gb_UnpackedTarball_get_dir,lcms2/include) \
		&& export ZLIB_INCLUDE_DIR=$(OUTDIR)/inc/external/zlib \
		&& export ICU_INCLUDE_DIR=$(OUTDIR)/inc/external \
		&& msbuild.exe libcdr.vcxproj /p:Configuration=Release \
	,build/win32)
else
$(call gb_ExternalProject_get_state_target,libcdr,build) :
	$(call gb_ExternalProject_run,build,\
		export LIBWPD_INCLUDE_DIR=$(OUTDIR)/inc/external \
		&& export LIBWPG_INCLUDE_DIR=$(OUTDIR)/inc/external \
		&& export LCMS2_INCLUDE_DIR=$(call gb_UnpackedTarball_get_dir,lcms2/include) \
		&& export ZLIB_INCLUDE_DIR=$(OUTDIR)/inc/external/zlib \
		&& export ICU_INCLUDE_DIR=$(OUTDIR)/inc/external \
		&& msbuild.exe libcdr.vcxproj /p:PlatformToolset=v110 /p:VisualStudioVersion=11.0 /p:Configuration=Release \
	,build/win32)
endif

else

$(call gb_ExternalProject_get_state_target,libcdr,build) :
	$(call gb_ExternalProject_run,build,\
		export PKG_CONFIG="" \
		&& export ICU_LIBS=" " \
		$(if $(filter NO,$(SYSTEM_ICU)),&& export ICU_CFLAGS="-I$(OUTDIR)/inc/external") \
		$(if $(filter YES,$(SYSTEM_ICU)),&& export ICU_CFLAGS=" ") \
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
