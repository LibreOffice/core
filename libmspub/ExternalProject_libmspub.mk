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

$(eval $(call gb_ExternalProject_use_autoconf,libmspub,build))

$(eval $(call gb_ExternalProject_register_targets,libmspub,\
	build \
))

$(eval $(call gb_ExternalProject_use_externals,libmspub,\
	boost_headers \
	icu \
	wpd \
	wpg \
))

$(call gb_ExternalProject_get_state_target,libmspub,build) :
	$(call gb_ExternalProject_run,build,\
		export PKG_CONFIG="" \
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
	)

# vim: set noet sw=4 ts=4:
