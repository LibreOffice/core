# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,cairo))

$(eval $(call gb_ExternalProject_use_external_project,cairo,pixman))

$(eval $(call gb_ExternalProject_use_externals,cairo,\
	freetype \
	png \
	zlib \
))

ifeq ($(OS),ANDROID)
$(eval $(call gb_ExternalProject_use_unpacked,cairo,fontconfig))
$(eval $(call gb_ExternalProject_use_unpacked,cairo,png))
endif

$(eval $(call gb_ExternalProject_register_targets,cairo,\
	build \
))

ifeq ($(OS)$(COM),WNTMSC)

$(call gb_ExternalProject_get_state_target,cairo,build) :
	$(call gb_ExternalProject_run,build,\
	$(MAKE) -f Makefile.win32 CFG=release ZLIB3RDLIB=zlib.lib \
	)


else

# overwrite src/cairo-version.h because that is just a dummy file and included
# from cairo.h in non-overridable way

$(call gb_ExternalProject_get_state_target,cairo,build) :
	$(call gb_ExternalProject_run,build,\
	./configure \
		$(if $(debug),STRIP=" ") \
		$(if $(filter ANDROID IOS,$(OS)),CFLAGS="$(if $(debug),-g) $(ZLIB_CFLAGS) $(gb_VISIBILITY_FLAGS)") \
		$(if $(filter-out ANDROID IOS,$(OS)),CFLAGS="$(if $(debug),-g) $(ZLIB_CFLAGS)" ) \
		$(if $(filter ANDROID IOS,$(OS)),PKG_CONFIG=./dummy_pkg_config) \
		LDFLAGS="$(ZLIB_LIBS)" \
		pixman_CFLAGS="-I$(call gb_UnpackedTarball_get_dir,pixman)/pixman" \
		pixman_LIBS="-L$(call gb_UnpackedTarball_get_dir,pixman)/pixman/.libs -lpixman-1" \
		png_REQUIRES="trick_configure_into_using_png_CFLAGS_and_LIBS" \
		png_CFLAGS="$(LIBPNG_CFLAGS)" png_LIBS="$(LIBPNG_LIBS)" \
		$(if $(SYSTEM_FREETYPE),,FREETYPE_CFLAGS="-I$(call gb_UnpackedTarball_get_dir,freetype)/include") \
		$(if $(SYSTEM_FONTCONFIG),,FONTCONFIG_CFLAGS="-I$(call gb_UnpackedTarball_get_dir,fontconfig)") \
		$(if $(filter TRUE,$(DISABLE_DYNLOADING)),--disable-shared,$(if $(filter ANDROID,$(OS)),--disable-shared,--disable-static)) \
		$(if $(filter ANDROID IOS,$(OS)),--disable-xlib --disable-xcb,$(if $(filter TRUE,$(ENABLE_HEADLESS)),--disable-xlib --disable-xcb,--enable-xlib --enable-xcb)) \
		$(if $(filter IOS,$(OS)),--enable-quartz --enable-quartz-font) \
		--disable-valgrind \
		$(if $(filter IOS,$(OS)),--disable-ft,--enable-ft --enable-fc) \
		--disable-svg --enable-gtk-doc=no --enable-test-surfaces=no \
		$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM) \
		$(if $(filter INTEL ARM,$(CPUNAME)),ac_cv_c_bigendian=no ax_cv_c_float_words_bigendian=no)) \
		$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
	&& cp cairo-version.h src/cairo-version.h \
	&& cd src && $(MAKE) \
	)

endif

# vim: set noet sw=4 ts=4:
