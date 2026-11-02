# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

pdfium_patches :=
# Fixes build on our baseline.
pdfium_patches += build.patch.1
pdfium_patches += c++20-comparison.patch

pdfium_patches += freebsd.patch

pdfium_patches += system-abseil.diff
# expose this mapping information
pdfium_patches += tounicodeinfo.patch.1
pdfium_patches += charcodetoglyphindex.patch.1
pdfium_patches += getfontdictobjnum.patch.1

pdfium_patches += system-fast_float.diff

ifeq ($(OS),WNT)
pdfium_patches += pdfium-vs2019-arm64_no-__umulh.patch.1
endif

# TODO, attempt upstream
pdfium_patches += ofz451333752.patch
pdfium_patches += ofz464123310.patch
pdfium_patches += ofz503815636.patch
pdfium_patches += container-stream-concepts.patch
pdfium_patches += abseil-hashtablez-sampler.patch

$(eval $(call gb_UnpackedTarball_UnpackedTarball,pdfium))

$(eval $(call gb_UnpackedTarball_set_tarball,pdfium,$(PDFIUM_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,pdfium,0))

$(eval $(call gb_UnpackedTarball_add_patches,pdfium,\
    $(foreach patch,$(pdfium_patches),external/pdfium/$(patch)) \
))

# hwy/per_target.cc includes itself via HWY_TARGET_INCLUDE, so copy that instead of moving it.
$(eval $(call gb_UnpackedTarball_set_post_action,pdfium,\
    mv third_party/bigint/BigInteger.cc third_party/bigint/BigInteger.cpp && \
    mv third_party/bigint/BigIntegerUtils.cc third_party/bigint/BigIntegerUtils.cpp && \
    mv third_party/bigint/BigUnsigned.cc third_party/bigint/BigUnsigned.cpp && \
    mv third_party/bigint/BigUnsignedInABase.cc third_party/bigint/BigUnsignedInABase.cpp && \
    mv core/fxcrt/debug/alias.cc core/fxcrt/debug/alias.cpp && \
    mv core/fxcrt/win/win_util.cc core/fxcrt/win/win_util.cpp && \
    mv third_party/libopenjpeg/opj_malloc.cc third_party/libopenjpeg/opj_malloc.cpp && \
    mv third_party/abseil-cpp/absl/base/throw_delegate.cc third_party/abseil-cpp/absl/base/throw_delegate.cpp && \
    mv third_party/abseil-cpp/absl/container/internal/raw_hash_set.cc third_party/abseil-cpp/absl/container/internal/raw_hash_set.cpp && \
    mv third_party/abseil-cpp/absl/base/internal/raw_logging.cc third_party/abseil-cpp/absl/base/internal/raw_logging.cpp && \
    mv third_party/abseil-cpp/absl/hash/internal/hash.cc third_party/abseil-cpp/absl/hash/internal/hash.cpp && \
    mv third_party/abseil-cpp/absl/hash/internal/city.cc third_party/abseil-cpp/absl/hash/internal/city.cpp && \
    mv third_party/highway/src/hwy/abort.cc third_party/highway/src/hwy/abort.cpp && \
    mv third_party/highway/src/hwy/aligned_allocator.cc third_party/highway/src/hwy/aligned_allocator.cpp && \
    cp third_party/highway/src/hwy/per_target.cc third_party/highway/src/hwy/per_target.cpp && \
    mv third_party/highway/src/hwy/print.cc third_party/highway/src/hwy/print.cpp && \
    mv third_party/highway/src/hwy/stats.cc third_party/highway/src/hwy/stats.cpp && \
    mv third_party/highway/src/hwy/targets.cc third_party/highway/src/hwy/targets.cpp && \
    mv third_party/highway/src/hwy/timer.cc third_party/highway/src/hwy/timer.cpp \
))

# vim: set noet sw=4 ts=4:
