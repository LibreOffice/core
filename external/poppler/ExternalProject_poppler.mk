# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,poppler))

$(eval $(call gb_ExternalProject_use_autoconf,poppler,build))

$(eval $(call gb_ExternalProject_register_targets,poppler,\
	build \
))

$(call gb_ExternalProject_get_state_target,poppler,build) :
	$(call gb_ExternalProject_run,build,\
		$(if $(filter TRUE,$(DISABLE_DYNLOADING)),CFLAGS="$(CFLAGS) $(gb_VISIBILITY_FLAGS) $(gb_COMPILEROPTFLAGS)" CXXFLAGS="$(CXXFLAGS) $(gb_VISIBILITY_FLAGS) $(gb_COMPILEROPTFLAGS)") \
		./configure \
			--with-pic \
			--enable-static \
			--disable-shared \
			--enable-xpdf-headers \
			--disable-poppler-cpp \
			--disable-libopenjpeg \
			--disable-libtiff \
			--disable-libjpeg \
			--disable-libpng \
			--disable-zlib \
			--disable-libcurl \
			--disable-splash-output \
			--disable-cairo-output \
			--disable-poppler-glib \
			--disable-poppler-qt4 \
			--disable-gtk-test \
			--disable-utils \
			--disable-cms \
			$(if $(filter WNT MACOSX,$(OS)),--with-font-configuration=win32,--with-font-configuration=fontconfig) \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
		&& $(MAKE) \
	)

# vim: set noet sw=4 ts=4:
