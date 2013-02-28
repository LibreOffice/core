# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,cairo))

$(eval $(call gb_ExternalProject_use_unpacked,cairo,cairo))

$(eval $(call gb_ExternalProject_use_external_project,cairo,pixman))

$(eval $(call gb_ExternalProject_register_targets,cairo,\
	build \
))

ifeq ($(OS)$(COM),WNTMSC)

$(call gb_ExternalProject_get_state_target,cairo,build) :
	cd $(EXTERNAL_WORKDIR) \
	&& $(MAKE) -f Makefile.win32 CFG=release ZLIB3RDLIB=zlib.lib \
	&& touch $@

else

# overwrite src/cairo-version.h because that is just a dummy file and included
# from cairo.h in non-overridable way

$(call gb_ExternalProject_get_state_target,cairo,build) :
	cd $(EXTERNAL_WORKDIR) \
	&& ./configure \
		$(if $(debug),STRIP=" ") \
		CFLAGS="$(if $(debug),-g) $(SOLARINC)" \
		LDFLAGS='-L$(OUTDIR)/lib' \
		$(if $(filter ANDROID IOS,$(OS)),PKG_CONFIG=./dummy_pkg_config) \
		pixman_CFLAGS="-I$(call gb_UnpackedTarball_get_dir,pixman)/pixman" \
		pixman_LIBS="-L$(call gb_UnpackedTarball_get_dir,pixman)/pixman/.libs -lpixman-1" \
		COMPRESS=$(if $(filter YES,$(SYSTEM_ZLIB)),compress,z_compress) \
		ZLIB3RDLIB=-lz \
		$(if $(filter NO,$(SYSTEM_LIBPNG)),png_CFLAGS="-I$(OUTDIR)/inc/external/libpng" png_LIBS="-L$(OUTDIR)/lib -lpng") \
		$(if $(filter IOS,$(OS)),--disable-shared,--disable-static) \
		$(if $(filter ANDROID IOS,$(OS)),--disable-xlib,--enable-xlib) \
		$(if $(filter IOS,$(OS)),--enable-quartz --enable-quartz-font) \
		--disable-valgrind \
		$(if $(filter IOS,$(OS)),--disable-ft,--enable-ft --enable-fc) \
		--disable-svg --enable-gtk-doc=no --enable-test-surfaces=no \
		$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
		$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
	&& cp cairo-version.h src/cairo-version.h \
	&& cd src && $(MAKE) \
	&& touch $@

endif

# vim: set noet sw=4 ts=4:
