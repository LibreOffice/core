# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,cairo))

$(eval $(call gb_ExternalProject_register_targets,cairo,\
	build \
))

$(eval $(call gb_ExternalProject_use_external_project,cairo,pixman))

$(eval $(call gb_ExternalProject_use_externals,cairo,\
	fontconfig \
	freetype \
	libpng \
	meson \
	zlib \
	libxml2 \
))

# We cannot use environment vars inside the meson cross-build file,
# so we're going to have to generate one on-the-fly.
# mungle variables into python list format
python_listify = '$(subst $(WHITESPACE),'$(COMMA)',$(strip $(1)))'
cross_c = $(call python_listify,$(gb_CC))
cross_cxx = $(call python_listify,$(gb_CXX))
cross_ld := $(call python_listify,$(subst -fuse-ld=,,$(USE_LD)))

define gb_cairo_cross_compile
[binaries]
c = [$(cross_c)]
cpp = [$(cross_cxx)]
c_ld = [$(cross_ld)]
cpp_ld = [$(cross_ld)]
ar = '$(AR)'
strip = '$(STRIP)'
# TODO: this is pretty ugly...
[host_machine]
system = '$(if $(filter WNT,$(OS)),windows,$(if $(filter MACOSX,$(OS)),darwin,$(if $(filter ANDROID,$(OS)),android,linux)))'
cpu_family = '$(subst X86_64,x86_64,$(RTL_ARCH))'
cpu = '$(if $(filter x86,$(RTL_ARCH)),i686,$(if $(filter X86_64,$(RTL_ARCH)),x86_64,$(if $(filter AARCH64,$(RTL_ARCH)),aarch64,armv7)))'
endian = '$(ENDIANNESS)'
endef

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

# We control how cairo handles its subprojects/dependencies by adding entries to PKG_CONFIG_PATH .
# Meson will look in those locations for *.pc files, which contains FLAGS and LIBS entries.

# We set PKG_CONFIG_TOP_BUILD_DIR= in order to work around some pkg-config weirdness, which otherwise
# will make meson crash.

$(call gb_ExternalProject_get_state_target,cairo,build) :
	$(call gb_Trace_StartRange,cairo,EXTERNAL)
	$(file >$(gb_UnpackedTarball_workdir)/cairo/cross-file.txt,$(gb_cairo_cross_compile))
	$(call gb_ExternalProject_run,build,\
		PYTHONWARNINGS= \
		PKG_CONFIG_TOP_BUILD_DIR= \
		$(if $(filter ANDROID iOS,$(OS)),CFLAGS="$(if $(debug),-g) $(gb_VISIBILITY_FLAGS)") \
		$(if $(filter EMSCRIPTEN,$(OS)),CFLAGS="-O3 -DCAIRO_NO_MUTEX -Wno-enum-conversion $(gb_EMSCRIPTEN_CPPFLAGS)" ) \
		PKG_CONFIG_PATH="${PKG_CONFIG_PATH}:$(gb_UnpackedTarball_workdir)/pixman/builddir/meson-uninstalled" \
		$(if $(SYSTEM_FREETYPE),,PKG_CONFIG_PATH="$$PKG_CONFIG_PATH:$(gb_UnpackedTarball_workdir)/freetype/instdir/lib/pkgconfig") \
		$(if $(SYSTEM_FONTCONFIG),,PKG_CONFIG_PATH="$$PKG_CONFIG_PATH:$(gb_UnpackedTarball_workdir)/fontconfig") \
		$(comment # on android we use system provided libz, but there is no .pc file, so create one for cairo) \
		$(if $(filter ANDROID,$(OS)), \
			$(shell mkdir $(gb_UnpackedTarball_workdir)/cairo/missing-system-pc) \
			$(file >$(gb_UnpackedTarball_workdir)/cairo/missing-system-pc/zlib.pc,$(call gb_pkgconfig_file,zlib,1.2.8,$(ZLIB_CFLAGS),$(ZLIB_LIBS))) \
			PKG_CONFIG_PATH="$$PKG_CONFIG_PATH:$(gb_UnpackedTarball_workdir)/cairo/missing-system-pc" \
		) \
		$(if $(SYSTEM_ZLIB),,PKG_CONFIG_PATH="$$PKG_CONFIG_PATH:$(gb_UnpackedTarball_workdir)/zlib") \
		$(if $(SYSTEM_LIBPNG),,PKG_CONFIG_PATH="$$PKG_CONFIG_PATH:$(gb_UnpackedTarball_workdir)/libpng") \
		$(if $(SYSTEM_LIBXML),,PKG_CONFIG_PATH="$$PKG_CONFIG_PATH:$(gb_UnpackedTarball_workdir)/libxml2") \
		$(if $(filter -fsanitize=undefined,$(CC)),CC='$(CC) -fno-sanitize=function') \
		$(MESON) setup --wrap-mode nofallback builddir \
			$(if $(debug),-Dstrip=false,-Dstrip=true) \
			$(if $(filter -fsanitize=%,$(CC)),-Db_lundef=false) \
			$(if $(filter TRUE,$(DISABLE_DYNLOADING)),-Ddefault_library=static,-Ddefault_library=shared) \
			$(if $(filter EMSCRIPTEN ANDROID iOS,$(OS)),-Dxlib=disabled -Dxcb=disabled,$(if $(filter TRUE,$(DISABLE_GUI)),-Dxlib=disabled -Dxcb=disabled,-Dxlib=enabled -Dxcb=enabled)) \
			$(if $(filter iOS,$(OS)),-Dquartz=enabled) \
			$(if $(filter iOS,$(OS)),-Dfreetype=disabled,-Dfreetype=enabled -Dfontconfig=enabled) \
			-Dgtk_doc=false -Dtests=disabled \
			$(if $(CROSS_COMPILING),--cross-file cross-file.txt) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
			$(if $(filter-out $(BUILD_PLATFORM),$(HOST_PLATFORM))$(WSL),--cross-file cross-file.txt) && \
		$(MESON) compile -C builddir \
			$(if $(verbose),--verbose) \
	)
	$(call gb_Trace_EndRange,cairo,EXTERNAL)

# vim: set noet sw=4 ts=4:
