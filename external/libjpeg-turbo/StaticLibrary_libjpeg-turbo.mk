# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozillarg/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,libjpeg-turbo))

$(eval $(call gb_StaticLibrary_use_unpacked,libjpeg-turbo,libjpeg-turbo))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,libjpeg-turbo))

$(eval $(call gb_StaticLibrary_set_include,libjpeg-turbo,\
    -I$(gb_UnpackedTarball_workdir)/libjpeg-turbo \
    $$(INCLUDE) \
))

ifeq ($(OS),WNT)
$(eval $(call gb_StaticLibrary_add_cflags,libjpeg-turbo,\
    -DHAVE_INTRIN_H=1 \
))
endif

$(eval $(call gb_StaticLibrary_add_cflags,libjpeg-turbo,\
    -DSIZEOF_SIZE_T=$(SIZEOF_SIZE_T) \
))

ifeq ($(COM),GCC)
ifneq ($(ENABLE_OPTIMIZED),)
$(eval $(call gb_StaticLibrary_add_cflags,libjpeg-turbo,\
    -O3 \
))
endif
endif

$(eval $(call gb_StaticLibrary_add_generated_cobjects,libjpeg-turbo,\
    UnpackedTarball/libjpeg-turbo/jaricom \
    UnpackedTarball/libjpeg-turbo/jcapimin \
    UnpackedTarball/libjpeg-turbo/jcapistd \
    UnpackedTarball/libjpeg-turbo/jcarith \
    UnpackedTarball/libjpeg-turbo/jccoefct \
    UnpackedTarball/libjpeg-turbo/jccolor \
    UnpackedTarball/libjpeg-turbo/jcdctmgr \
    UnpackedTarball/libjpeg-turbo/jchuff \
    UnpackedTarball/libjpeg-turbo/jcicc \
    UnpackedTarball/libjpeg-turbo/jcinit \
    UnpackedTarball/libjpeg-turbo/jcmainct \
    UnpackedTarball/libjpeg-turbo/jcmarker \
    UnpackedTarball/libjpeg-turbo/jcmaster \
    UnpackedTarball/libjpeg-turbo/jcomapi \
    UnpackedTarball/libjpeg-turbo/jcparam \
    UnpackedTarball/libjpeg-turbo/jcphuff \
    UnpackedTarball/libjpeg-turbo/jcprepct \
    UnpackedTarball/libjpeg-turbo/jcsample \
    UnpackedTarball/libjpeg-turbo/jctrans \
    UnpackedTarball/libjpeg-turbo/jdapimin \
    UnpackedTarball/libjpeg-turbo/jdapistd \
    UnpackedTarball/libjpeg-turbo/jdarith \
    UnpackedTarball/libjpeg-turbo/jdatadst \
    UnpackedTarball/libjpeg-turbo/jdatasrc \
    UnpackedTarball/libjpeg-turbo/jdcoefct \
    UnpackedTarball/libjpeg-turbo/jdcolor \
    UnpackedTarball/libjpeg-turbo/jddctmgr \
    UnpackedTarball/libjpeg-turbo/jdhuff \
    UnpackedTarball/libjpeg-turbo/jdicc \
    UnpackedTarball/libjpeg-turbo/jdinput \
    UnpackedTarball/libjpeg-turbo/jdmainct \
    UnpackedTarball/libjpeg-turbo/jdmarker \
    UnpackedTarball/libjpeg-turbo/jdmaster \
    UnpackedTarball/libjpeg-turbo/jdmerge \
    UnpackedTarball/libjpeg-turbo/jdphuff \
    UnpackedTarball/libjpeg-turbo/jdpostct \
    UnpackedTarball/libjpeg-turbo/jdsample \
    UnpackedTarball/libjpeg-turbo/jdtrans \
    UnpackedTarball/libjpeg-turbo/jerror \
    UnpackedTarball/libjpeg-turbo/jfdctflt \
    UnpackedTarball/libjpeg-turbo/jfdctfst \
    UnpackedTarball/libjpeg-turbo/jfdctint \
    UnpackedTarball/libjpeg-turbo/jidctflt \
    UnpackedTarball/libjpeg-turbo/jidctfst \
    UnpackedTarball/libjpeg-turbo/jidctint \
    UnpackedTarball/libjpeg-turbo/jidctred \
    UnpackedTarball/libjpeg-turbo/jmemmgr \
    UnpackedTarball/libjpeg-turbo/jmemnobs \
    UnpackedTarball/libjpeg-turbo/jquant1 \
    UnpackedTarball/libjpeg-turbo/jquant2 \
    UnpackedTarball/libjpeg-turbo/jutils \
))

ifneq ($(NASM),)

$(eval $(call gb_StaticLibrary_add_nasmflags,libjpeg-turbo,\
	-I$(gb_UnpackedTarball_workdir)/libjpeg-turbo/simd/nasm/ \
	-I$(dir $(gb_UnpackedTarball_workdir)/libjpeg-turbo/$(1)) \
))

ifeq ($(CPUNAME),X86_64)

$(eval $(call gb_StaticLibrary_add_cflags,libjpeg-turbo,\
    -DWITH_SIMD \
))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,libjpeg-turbo,\
    UnpackedTarball/libjpeg-turbo/simd/x86_64/jsimd \
))

$(eval $(call gb_StaticLibrary_add_generated_nasmobjects,libjpeg-turbo,\
    UnpackedTarball/libjpeg-turbo/simd/x86_64/jsimdcpu.asm \
    UnpackedTarball/libjpeg-turbo/simd/x86_64/jfdctflt-sse.asm \
    UnpackedTarball/libjpeg-turbo/simd/x86_64/jccolor-sse2.asm \
    UnpackedTarball/libjpeg-turbo/simd/x86_64/jcgray-sse2.asm \
    UnpackedTarball/libjpeg-turbo/simd/x86_64/jchuff-sse2.asm \
    UnpackedTarball/libjpeg-turbo/simd/x86_64/jcphuff-sse2.asm \
    UnpackedTarball/libjpeg-turbo/simd/x86_64/jcsample-sse2.asm \
    UnpackedTarball/libjpeg-turbo/simd/x86_64/jdcolor-sse2.asm \
    UnpackedTarball/libjpeg-turbo/simd/x86_64/jdmerge-sse2.asm \
    UnpackedTarball/libjpeg-turbo/simd/x86_64/jdsample-sse2.asm \
    UnpackedTarball/libjpeg-turbo/simd/x86_64/jfdctfst-sse2.asm \
    UnpackedTarball/libjpeg-turbo/simd/x86_64/jfdctint-sse2.asm \
    UnpackedTarball/libjpeg-turbo/simd/x86_64/jidctflt-sse2.asm \
    UnpackedTarball/libjpeg-turbo/simd/x86_64/jidctfst-sse2.asm \
    UnpackedTarball/libjpeg-turbo/simd/x86_64/jidctint-sse2.asm \
    UnpackedTarball/libjpeg-turbo/simd/x86_64/jidctred-sse2.asm \
    UnpackedTarball/libjpeg-turbo/simd/x86_64/jquantf-sse2.asm \
    UnpackedTarball/libjpeg-turbo/simd/x86_64/jquanti-sse2.asm \
    UnpackedTarball/libjpeg-turbo/simd/x86_64/jccolor-avx2.asm \
    UnpackedTarball/libjpeg-turbo/simd/x86_64/jcgray-avx2.asm \
    UnpackedTarball/libjpeg-turbo/simd/x86_64/jcsample-avx2.asm \
    UnpackedTarball/libjpeg-turbo/simd/x86_64/jdcolor-avx2.asm \
    UnpackedTarball/libjpeg-turbo/simd/x86_64/jdmerge-avx2.asm \
    UnpackedTarball/libjpeg-turbo/simd/x86_64/jdsample-avx2.asm \
    UnpackedTarball/libjpeg-turbo/simd/x86_64/jfdctint-avx2.asm \
    UnpackedTarball/libjpeg-turbo/simd/x86_64/jidctint-avx2.asm \
    UnpackedTarball/libjpeg-turbo/simd/x86_64/jquanti-avx2.asm \
))

else ifeq ($(CPUNAME),INTEL)

$(eval $(call gb_StaticLibrary_add_cflags,libjpeg-turbo,\
    -DWITH_SIMD \
))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,libjpeg-turbo,\
    UnpackedTarball/libjpeg-turbo/simd/i386/jsimd \
))

$(eval $(call gb_StaticLibrary_add_generated_nasmobjects,libjpeg-turbo,\
    UnpackedTarball/libjpeg-turbo/simd/i386/jsimdcpu.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jfdctflt-3dn.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jidctflt-3dn.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jquant-3dn.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jccolor-mmx.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jcgray-mmx.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jcsample-mmx.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jdcolor-mmx.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jdmerge-mmx.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jdsample-mmx.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jfdctfst-mmx.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jfdctint-mmx.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jidctfst-mmx.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jidctint-mmx.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jidctred-mmx.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jquant-mmx.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jfdctflt-sse.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jidctflt-sse.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jquant-sse.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jccolor-sse2.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jcgray-sse2.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jchuff-sse2.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jcphuff-sse2.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jcsample-sse2.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jdcolor-sse2.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jdmerge-sse2.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jdsample-sse2.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jfdctfst-sse2.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jfdctint-sse2.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jidctflt-sse2.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jidctfst-sse2.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jidctint-sse2.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jidctred-sse2.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jquantf-sse2.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jquanti-sse2.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jccolor-avx2.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jcgray-avx2.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jcsample-avx2.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jdcolor-avx2.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jdmerge-avx2.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jdsample-avx2.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jfdctint-avx2.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jidctint-avx2.asm \
    UnpackedTarball/libjpeg-turbo/simd/i386/jquanti-avx2.asm \
))

else

$(eval $(call gb_StaticLibrary_add_generated_cobjects,libjpeg-turbo,\
    UnpackedTarball/libjpeg-turbo/jsimd_none \
))

endif

else

$(eval $(call gb_StaticLibrary_add_generated_cobjects,libjpeg-turbo,\
    UnpackedTarball/libjpeg-turbo/jsimd_none \
))

endif


# vim: set noet sw=4 ts=4:
