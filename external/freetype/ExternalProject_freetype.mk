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

$(call gb_ExternalProject_get_state_target,freetype,build) :
	cd $(EXTERNAL_WORKDIR) \
	&& CFLAGS="$(if $(debug),-g) $(gb_VISIBILITY_FLAGS)" \
		./configure \
		--disable-shared \
		--without-zlib \
		--without-bzip2 \
		--prefix=$(OUTDIR) --includedir=$(call gb_UnpackedTarball_get_dir,freetype)/include \
		--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM) \
	&& chmod +x builds/unix/freetype-config \
	&& $(MAKE) \
	&& touch $@

# vim: set noet sw=4 ts=4:
