# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

pdfium_patches :=
pdfium_patches += visibility.patch.1
pdfium_patches += ubsan.patch
# Fixes build on our baseline.
pdfium_patches += build.patch.1
# Adds missing editing API
pdfium_patches += 0002-svx-more-accurate-PDF-text-importing.patch.2
pdfium_patches += 0003-svx-import-PDF-images-as-BGRA.patch.2
pdfium_patches += 0004-svx-support-PDF-text-color.patch.2
pdfium_patches += 0009-svx-support-color-text-for-imported-PDFs.patch.2
pdfium_patches += 0010-svx-support-importing-forms-from-PDFs.patch.2
pdfium_patches += 0011-svx-correctly-possition-form-objects-from-PDF.patch.2
pdfium_patches += 0012-svx-import-processed-PDF-text.patch.2
pdfium_patches += 0014-svx-update-PDFium-patch-and-code.patch.2
pdfium_patches += 0015-svx-set-the-font-name-of-imported-PDF-text.patch.2

$(eval $(call gb_UnpackedTarball_UnpackedTarball,pdfium))

$(eval $(call gb_UnpackedTarball_set_tarball,pdfium,$(PDFIUM_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,pdfium,0))

$(eval $(call gb_UnpackedTarball_add_patches,pdfium,\
    $(foreach patch,$(pdfium_patches),external/pdfium/$(patch)) \
))

# Upstream build system generates this, but it's not really a generated file.
# Stripped down version from <https://dxr.mozilla.org/mozilla-central/source/ipc/chromium/src/build/build_config.h>.
$(eval $(call gb_UnpackedTarball_add_file,pdfium,build/build_config.h,external/pdfium/configs/build_config.h))

$(eval $(call gb_UnpackedTarball_set_post_action,pdfium,\
    mv third_party/bigint/BigInteger.cc third_party/bigint/BigInteger.cpp && \
    mv third_party/bigint/BigIntegerUtils.cc third_party/bigint/BigIntegerUtils.cpp && \
    mv third_party/bigint/BigUnsigned.cc third_party/bigint/BigUnsigned.cpp && \
    mv third_party/bigint/BigUnsignedInABase.cc third_party/bigint/BigUnsignedInABase.cpp && \
    mv third_party/base/allocator/partition_allocator/address_space_randomization.cc third_party/base/allocator/partition_allocator/address_space_randomization.cpp && \
    mv third_party/base/allocator/partition_allocator/page_allocator.cc third_party/base/allocator/partition_allocator/page_allocator.cpp && \
    mv third_party/base/allocator/partition_allocator/partition_alloc.cc third_party/base/allocator/partition_allocator/partition_alloc.cpp && \
    mv third_party/base/allocator/partition_allocator/spin_lock.cc third_party/base/allocator/partition_allocator/spin_lock.cpp \
))

# vim: set noet sw=4 ts=4:
