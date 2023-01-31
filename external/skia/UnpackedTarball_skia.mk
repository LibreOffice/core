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
    clang11-flax-vector-conversion.patch.0 \
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
    fast-png-write.patch.1 \
    skia_sk_cpu_sse_level_0_by_default.patch.1 \
    fix-warnings.patch.1 \
    windows-libraries-system32.patch.1 \
    fix-graphite-ifdef.patch.1 \
    allow-no-es2restrictions.patch.1 \
    vk_mem_alloc.patch.1 \
    tdf148624.patch.1 \
    constexpr-template.patch.0 \
    missing-include.patch.0 \
    tdf147342.patch.0 \

$(eval $(call gb_UnpackedTarball_set_patchlevel,skia,1))

$(eval $(call gb_UnpackedTarball_add_patches,skia,\
    $(foreach patch,$(skia_patches),external/skia/$(patch)) \
))

$(eval $(call gb_UnpackedTarball_set_post_action,skia,\
    mv third_party/skcms/skcms.cc third_party/skcms/skcms.cpp \
))

# vim: set noet sw=4 ts=4:
