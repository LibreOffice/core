# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,libpng))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,libpng))

$(eval $(call gb_StaticLibrary_use_unpacked,libpng,libpng))

$(eval $(call gb_StaticLibrary_use_externals,libpng,\
	zlib \
))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,libpng,\
	UnpackedTarball/libpng/png \
	UnpackedTarball/libpng/pngerror \
	UnpackedTarball/libpng/pngget \
	UnpackedTarball/libpng/pngmem \
	UnpackedTarball/libpng/pngpread \
	UnpackedTarball/libpng/pngread \
	UnpackedTarball/libpng/pngrio \
	UnpackedTarball/libpng/pngrtran \
	UnpackedTarball/libpng/pngrutil \
	UnpackedTarball/libpng/pngset \
	UnpackedTarball/libpng/pngtrans \
	UnpackedTarball/libpng/pngwio \
	UnpackedTarball/libpng/pngwrite \
	UnpackedTarball/libpng/pngwtran \
	UnpackedTarball/libpng/pngwutil \
	$(if $(filter ARM AARCH64 ARM64,$(CPUNAME)),\
	    UnpackedTarball/libpng/arm/arm_init \
	    UnpackedTarball/libpng/arm/filter_neon_intrinsics \
	    UnpackedTarball/libpng/arm/palette_neon_intrinsics \
	) \
	$(if $(filter POWERPC POWERPC64,$(CPUNAME)), \
	    UnpackedTarball/libpng/powerpc/powerpc_init \
	    UnpackedTarball/libpng/powerpc/filter_vsx_intrinsics \
	) \
	$(if $(filter INTEL X86_64,$(CPUNAME)), \
	    UnpackedTarball/libpng/intel/intel_init \
	    UnpackedTarball/libpng/intel/filter_sse2_intrinsics \
	) \
))

$(eval $(call gb_StaticLibrary_add_defs,libpng,\
	$(if $(filter ARM AARCH64 ARM64,$(CPUNAME)), -DPNG_ARM_NEON) \
	$(if $(filter POWERPC POWERPC64,$(CPUNAME)), -DPNG_POWERPC_VSX ) \
	$(if $(filter INTEL X86_64,$(CPUNAME)), -DPNG_INTEL_SSE_OPT) \
))

# At least on Linux, with --enable-lto, when building both this external/libpng and external/skia,
# and building with GCC but building skia with Clang (which should be the sole combination that
# matches "LO_CLANG_CC is non-empty"), build this as a fat archive (including both the intermediate GCC
# object code for LTO and machine object code).  Besides targets like Library_vcl (which benefit
# from the  intermediate GCC object code for LTO), also Library_skia (built with the Clang toolchain
# lld, which does not understand intermediate GCC object code) includes this, so would otherwise
# fail to link (but now does not benefit from LTO for this included StaticLibrary_libpng):
ifeq ($(OS)-$(ENABLE_LTO),LINUX-TRUE)
ifneq ($(filter SKIA,$(BUILD_TYPE)),)
ifneq ($(LO_CLANG_CC),)
$(eval $(call gb_StaticLibrary_add_cflags,libpng,-ffat-lto-objects))
endif
endif
endif

# vim: set noet sw=4 ts=4:
