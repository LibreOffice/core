# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libtiff))

$(eval $(call gb_ExternalProject_register_targets,libtiff,\
	build \
))

$(eval $(call gb_ExternalProject_use_externals,libtiff,\
    libjpeg \
    libwebp \
    zlib \
))

$(eval $(call gb_ExternalProject_use_autoconf,libtiff,build))

# using ac_cv_lib_z_inflateEnd=yes to skip test for our
# static windows lib where the name is zlib not z
# using ac_cv_lib_jpeg_jpeg_read_scanlines and
# ac_cv_lib_jpeg_jpeg12_read_scanlines to skip tests
# for our static jpeg lib where the name is libjpeg-turbo.lib
# or liblibjpeg-turbo.a not libjpeg.lib/libjpeg.a
# we're building this statically anyway so the lib isn't
# used during the link done here

$(call gb_ExternalProject_get_state_target,libtiff,build) :
	$(call gb_Trace_StartRange,libtiff,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		export PKG_CONFIG="" \
		&& MAKE=$(MAKE) $(gb_RUN_CONFIGURE) ./configure \
			--enable-static \
			--enable-jpeg \
			--enable-zlib \
			--enable-webp \
			--disable-shared \
			--disable-cxx \
			--disable-libdeflate \
			--disable-jbig \
			--disable-lerc \
			--disable-lzma \
			--disable-mdi \
			--disable-win32-io \
			--disable-zstd \
			--with-pic \
			--without-x \
			$(if $(verbose),--disable-silent-rules,--enable-silent-rules) \
			CFLAGS="$(CFLAGS) $(call gb_ExternalProject_get_build_flags,libtiff) $(gb_EMSCRIPTEN_CFLAGS)" \
			$(if $(SYSTEM_ZLIB),,--with-zlib-include-dir="$(gb_UnpackedTarball_workdir)/zlib") \
			$(if $(SYSTEM_ZLIB),,--with-zlib-lib-dir="$(gb_StaticLibrary_WORKDIR)") \
			$(if $(SYSTEM_LIBJPEG),,--with-jpeg-include-dir="$(gb_UnpackedTarball_workdir)/libjpeg-turbo") \
			$(if $(SYSTEM_LIBJPEG),,--with-jpeg-lib-dir="$(gb_StaticLibrary_WORKDIR)") \
			$(if $(SYSTEM_LIBWEBP),,--with-webp-include-dir="$(gb_UnpackedTarball_workdir)/libwebp/src") \
			$(if $(SYSTEM_LIBWEBP),,$(if $(filter WNT,$(OS_FOR_BUILD)),\
				--with-webp-lib-dir="$(gb_UnpackedTarball_workdir)/libwebp/output/lib/libwebp$(if $(MSVC_USE_DEBUG_RUNTIME),_debug)$(gb_StaticLibrary_PLAINEXT)", \
				--with-webp-lib-dir="$(gb_UnpackedTarball_workdir)/libwebp/src/.libs")) \
			CPPFLAGS="$(CPPFLAGS) $(BOOST_CPPFLAGS) $(gb_EMSCRIPTEN_CPPFLAGS)" \
			LDFLAGS="$(call gb_ExternalProject_get_link_flags,libtiff) $(gb_EMSCRIPTEN_LDFLAGS)" \
			ac_cv_lib_z_inflateEnd=yes \
			ac_cv_lib_jpeg_jpeg_read_scanlines=yes \
			ac_cv_lib_jpeg_jpeg12_read_scanlines=no \
			ac_cv_lib_webp_WebPDecode=yes \
			$(gb_CONFIGURE_PLATFORMS) \
		&& cd libtiff && $(MAKE) libtiff.la \
	)
	$(call gb_Trace_EndRange,libtiff,EXTERNAL)

# vim: set noet sw=4 ts=4:
