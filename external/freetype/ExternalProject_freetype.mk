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
	$(call gb_Trace_StartRange,freetype,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		$(gb_RUN_CONFIGURE) ./configure \
			--disable-shared \
			--with-pic \
			--with-zlib \
			--without-brotli \
			--without-bzip2 \
			--without-harfbuzz \
			--without-png \
			--prefix=$(gb_UnpackedTarball_workdir)/freetype/instdir \
			$(gb_CONFIGURE_PLATFORMS) \
			$(if $(filter -fsanitize=undefined,$(CC)),CC='$(CC) -fno-sanitize=function') \
			CFLAGS="$(CFLAGS) \
				$(call gb_ExternalProject_get_build_flags,freetype) \
				$(call gb_ExternalProject_get_link_flags,freetype) \
				$(gb_VISIBILITY_FLAGS) \
				$(gb_EMSCRIPTEN_CPPFLAGS)" \
		&& $(MAKE) install \
		&& touch $@	)
	$(call gb_Trace_EndRange,freetype,EXTERNAL)

# vim: set noet sw=4 ts=4:
