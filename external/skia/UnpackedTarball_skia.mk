# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,skia))

$(eval $(call gb_UnpackedTarball_set_tarball,skia,$(SKIA_TARBALL)))

skia_patches := \
    fix-pch.patch.1 \
    fix-ddi.patch \
    make-api-visible.patch.1 \
    no-trace-resources-on-exit.patch.1 \
    fix-alpha-difference-copy.patch.1 \
    share-grcontext.patch.1 \
    clang-attributes-warning.patch.1 \
    fontconfig-get-typeface.patch.0 \
    windows-do-not-modify-logfont.patch.0 \
    windows-text-gamma.patch.0 \
    windows-force-unicode-api.patch.0 \
    fix-without-gl.patch.1 \
    windows-typeface-directwrite.patch.0 \
    windows-raster-surface-no-copies.patch.1 \
    fix-windows-dwrite.patch.1 \
    swap-buffers-rect.patch.1 \
    ubsan.patch.1 \
    fix-warnings.patch.1 \
    windows-libraries-system32.patch.1 \
    allow-no-es2restrictions.patch.1 \
    vk_mem_alloc.patch.1 \
    tdf147342.patch.0 \
    redefinition-of-op.patch.0 \
    0001-Added-missing-include-cstdio.patch \
    fix-SkDebugf-link-error.patch.1 \
    incomplete.patch.0 \
    ubsan-missing-typeinfo.patch.1 \
    incomplete-type-SkImageGenerator.patch.1 \
    0001-AvoidCombiningExtrememelyLargeMeshes.patch.1 \
    sort-comparison-assumption.patch.0 \

$(eval $(call gb_UnpackedTarball_set_patchlevel,skia,1))

ifneq ($(MSYSTEM),)
# use binary flag so patch from git-bash won't choke on mixed line-endings in patches
$(eval $(call gb_UnpackedTarball_set_patchflags,skia,--binary))
endif

$(eval $(call gb_UnpackedTarball_add_patches,skia,\
    $(foreach patch,$(skia_patches),external/skia/$(patch)) \
))

$(eval $(call gb_UnpackedTarball_set_post_action,skia,\
    mv modules/skcms/skcms.cc modules/skcms/skcms.cpp \
))

# vim: set noet sw=4 ts=4:
