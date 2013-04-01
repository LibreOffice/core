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

$(eval $(call gb_ExternalProject_use_autoconf,libcdr,build))

$(eval $(call gb_ExternalProject_register_targets,libcdr,\
	build \
))

$(eval $(call gb_ExternalProject_use_externals,libcdr,\
	icu \
	lcms2 \
	wpd \
	wpg \
	zlib \
))

$(call gb_ExternalProject_get_state_target,libcdr,build) :
	$(call gb_ExternalProject_run,build,\
		export PKG_CONFIG="" \
		&& export ICU_LIBS=" " \
		&& export ICU_CFLAGS="$(if $(filter NO,$(SYSTEM_ICU)),\
			-I$(call gb_UnpackedTarball_get_dir,icu)/source/i18n \
			-I$(call gb_UnpackedTarball_get_dir,icu)/source/common, )" \
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

# vim: set noet sw=4 ts=4:
