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
	libpng \
	zlib \
))

ifeq ($(OS),ANDROID)
$(eval $(call gb_ExternalProject_use_unpacked,cairo,fontconfig))
$(eval $(call gb_ExternalProject_use_unpacked,cairo,libpng))
endif

$(eval $(call gb_ExternalProject_register_targets,cairo,\
	build \
))

ifeq ($(OS),WNT)

$(call gb_ExternalProject_get_state_target,cairo,build) :
	$(call gb_Trace_StartRange,cairo,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
	$(MAKE) -f Makefile.win32 CFG=release ZLIB3RDLIB=zlib.lib \
	)
	$(call gb_Trace_EndRange,cairo,EXTERNAL)


else

# Including -rtlib=compiler-rt in pixman_LIBS is a BAD HACK:  At least when compiling with Clang
# -fsanitize=undefined on Linux x86-64, the generated code references __muloti4, which is an
# extension provided by libclang_rt.builtins-x86_64.a runtime, but not by GCC's libgcc_s.so.1 (which
# ultimately boils down to a bug in LLVM, I would say).  I am not sure whether it should in general
# work to mix uses of the (default on Linux, at least) GCC libgcc_s and LLVM's libclang_rt.builtins
# runtime libraries in one process, but for this specific case of libcairo.so it appears to work
# well:  For one, the only symbol referenced by libcairo.so from the runtime library is __muloti4;
# for another, at least in my LLVM build, lib/clang/12.0.0/lib/linux/libclang_rt.builtins-x86_64.a
# is only provided as a static archive; so libcairo.so will only contain a "harmless" copy of
# __muloti4 and not have a DT_NEEDED of any libclang_rt.builtins dynamic library that it would pull
# in at runtime.
# But passing -rtlib=compiler-rt into cairo's configure via the more obvious LDFLAGS would fail at
# least when building with -fsanitize=address and -fsanitize=undefined, as then the executable
# compiled  by configure when "checking whether the C compiler works" would reference
# _Unwind_Backtrace etc. that are provided by GCC's libgcc_s.so.1 but not by LLVM's
# libclang_rt.builtins-x86_64.a (and whatever the reason for that inconsistency).  So
# -rtlib=compiler-rt must be passed just into the linking of libcairo.so, but not generally into
# cairo's configure.  And pixman_LIBS happens to offer that.  (The -Wc is necessary so that libtool
# does not throw away the -rtlib=compiler-rt which it does not understand.)

# overwrite src/cairo-version.h because that is just a dummy file and included
# from cairo.h in non-overridable way

$(call gb_ExternalProject_get_state_target,cairo,build) :
	$(call gb_Trace_StartRange,cairo,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
	./configure \
		$(if $(debug),STRIP=" ") \
		$(if $(filter ANDROID iOS,$(OS)),CFLAGS="$(if $(debug),-g) $(ZLIB_CFLAGS) $(gb_VISIBILITY_FLAGS)") \
		$(if $(filter-out ANDROID iOS,$(OS)),CFLAGS="$(if $(debug),-g) $(ZLIB_CFLAGS)" ) \
		$(if $(filter ANDROID iOS,$(OS)),PKG_CONFIG=./dummy_pkg_config) \
		LIBS="$(ZLIB_LIBS)" \
		$(if $(filter -fsanitize=%,$(LDFLAGS)),LDFLAGS="$(LDFLAGS) -fuse-ld=bfd") \
		pixman_CFLAGS="-I$(call gb_UnpackedTarball_get_dir,pixman)/pixman" \
		pixman_LIBS="-L$(call gb_UnpackedTarball_get_dir,pixman)/pixman/.libs -lpixman-1 \
			$(if $(filter LINUX,$(OS)),-Wl$(COMMA)-z$(COMMA)origin \
					-Wl$(COMMA)-rpath$(COMMA)\\\$$\$$ORIGIN) \
			$(if $(filter -fsanitize=%,$(CC)), \
			    $(if $(filter LINUX-X86_64-TRUE,$(OS)-$(CPUNAME)-$(COM_IS_CLANG)), \
			        -Wc$(COMMA)-rtlib=compiler-rt))" \
		png_REQUIRES="trick_configure_into_using_png_CFLAGS_and_LIBS" \
		png_CFLAGS="$(LIBPNG_CFLAGS)" png_LIBS="$(LIBPNG_LIBS)" \
		$(if $(SYSTEM_FREETYPE),,FREETYPE_CFLAGS="-I$(call gb_UnpackedTarball_get_dir,freetype)/include") \
		$(if $(SYSTEM_FONTCONFIG),,FONTCONFIG_CFLAGS="-I$(call gb_UnpackedTarball_get_dir,fontconfig)") \
		$(if $(verbose),--disable-silent-rules,--enable-silent-rules) \
		$(if $(filter TRUE,$(DISABLE_DYNLOADING)),--disable-shared,$(if $(filter ANDROID,$(OS)),--disable-shared,--disable-static)) \
		$(if $(filter ANDROID iOS,$(OS)),--disable-xlib --disable-xcb,$(if $(filter TRUE,$(DISABLE_GUI)),--disable-xlib --disable-xcb,--enable-xlib --enable-xcb)) \
		$(if $(filter iOS,$(OS)),--enable-quartz --enable-quartz-font) \
		--disable-valgrind \
		$(if $(filter iOS,$(OS)),--disable-ft,--enable-ft --enable-fc) \
		--disable-svg --enable-gtk-doc=no --enable-test-surfaces=no \
		$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM) \
		$(if $(filter INTEL ARM,$(CPUNAME)),ac_cv_c_bigendian=no ax_cv_c_float_words_bigendian=no)) \
		$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
	&& cp cairo-version.h src/cairo-version.h \
	&& cd src && $(MAKE) \
	)
	$(call gb_Trace_EndRange,cairo,EXTERNAL)

endif

# vim: set noet sw=4 ts=4:
