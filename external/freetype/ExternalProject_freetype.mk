# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,freetype))

$(eval $(call gb_ExternalProject_register_targets,freetype,\
	build \
))

ifeq ($(OS)$(COM),WNTMSC)
$(call gb_ExternalProject_get_state_target,freetype,build) :
	$(call gb_ExternalProject_run,build,\
		cd ../builds/win32/vc2010/ && \
		msbuild.exe freetype.vcxproj /p:Configuration=$(if $(MSVC_USE_DEBUG_RUNTIME),Debug,Release) \
	,objs)
else
$(call gb_ExternalProject_get_state_target,freetype,build) :
	$(call gb_ExternalProject_run,build,\
		./configure \
			--disable-shared \
			--without-zlib \
			--without-bzip2 \
			--without-harfbuzz \
			--prefix=$(call gb_UnpackedTarball_get_dir,freetype/instdir) \
			--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM) \
			CFLAGS="$(if $(debug),-g) $(gb_VISIBILITY_FLAGS)" \
		&& $(MAKE) install \
		&& touch $@	)
endif

# vim: set noet sw=4 ts=4:
