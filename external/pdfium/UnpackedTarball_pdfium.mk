# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

pdfium_patches :=
pdfium_patches += ubsan.patch
# Fixes build on our baseline.
pdfium_patches += build.patch.1
# Avoids Windows 8 build dependency.
pdfium_patches += windows7.patch.1
pdfium_patches += c++20-comparison.patch
# Use CoreGraphics.h instead of Carbon.h -- https://pdfium-review.googlesource.com/c/pdfium/+/99753
# pdfium_patches += cg-instead-of-carbon.patch.1
# Android NDK 19 - that is known to work well - does not have 2 defines -- https://pdfium-review.googlesource.com/c/pdfium/+/96530
# pdfium_patches += AndroidNDK19.patch.1

pdfium_patches += include.patch

pdfium_patches += abseil-trivial.patch

pdfium_patches += constexpr-template.patch

$(eval $(call gb_UnpackedTarball_UnpackedTarball,pdfium))

$(eval $(call gb_UnpackedTarball_set_tarball,pdfium,$(PDFIUM_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,pdfium,0))

$(eval $(call gb_UnpackedTarball_add_patches,pdfium,\
    $(foreach patch,$(pdfium_patches),external/pdfium/$(patch)) \
))

$(eval $(call gb_UnpackedTarball_set_post_action,pdfium,\
    mv third_party/bigint/BigInteger.cc third_party/bigint/BigInteger.cpp && \
    mv third_party/bigint/BigIntegerUtils.cc third_party/bigint/BigIntegerUtils.cpp && \
    mv third_party/bigint/BigUnsigned.cc third_party/bigint/BigUnsigned.cpp && \
    mv third_party/bigint/BigUnsignedInABase.cc third_party/bigint/BigUnsignedInABase.cpp && \
    mv third_party/base/debug/alias.cc third_party/base/debug/alias.cpp && \
    mv third_party/base/memory/aligned_memory.cc third_party/base/memory/aligned_memory.cpp && \
    mv third_party/base/win/win_util.cc third_party/base/win/win_util.cpp && \
    mv third_party/libopenjpeg/opj_malloc.cc third_party/libopenjpeg/opj_malloc.cpp && \
    mv third_party/abseil-cpp/absl/types/bad_optional_access.cc third_party/abseil-cpp/absl/types/bad_optional_access.cpp && \
    mv third_party/abseil-cpp/absl/types/bad_variant_access.cc third_party/abseil-cpp/absl/types/bad_variant_access.cpp \
))

# vim: set noet sw=4 ts=4:
