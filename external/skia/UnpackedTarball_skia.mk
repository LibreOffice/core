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

# * c++20.patch.0 has been reported upstream at
#   <https://groups.google.com/forum/#!topic/skia-discuss/pYZQq_sLnv8> "C++20 operator== issue":
skia_patches := \
    lerp.patch \
    fix-pch.patch.1 \
    fix-ddi.patch \
    make-api-visible.patch.1 \
    no-trace-resources-on-exit.patch.1 \
    fix-alpha-difference-copy.patch.1 \
    libvulkan-name.patch.1 \
    share-grcontext.patch.1 \
    Wdeprecated-copy-dtor.patch.0 \
    fix-msvc.patch.1 \
    fix-gcc-x86.patch.1	\
    fix-sse.patch.1 \
    clang11-flax-vector-conversion.patch.0 \
    clang-attributes-warning.patch.1 \
    fontconfig-get-typeface.patch.0 \
    windows-do-not-modify-logfont.patch.0 \
    windows-text-gamma.patch.0 \
    windows-force-unicode-api.patch.0 \
    operator-eq-bool.patch.1 \
    fix-without-gl.patch.0 \
    windows-typeface-directwrite.patch.0 \
    windows-raster-surface-no-copies.patch.1 \
    fix-windows-dwrite.patch.1 \
    public-make-from-backend-texture.patch.1 \
    c++20.patch.0 \
    missing-include.patch.1 \

$(eval $(call gb_UnpackedTarball_set_patchlevel,skia,1))

$(eval $(call gb_UnpackedTarball_add_patches,skia,\
    $(foreach patch,$(skia_patches),external/skia/$(patch)) \
))

$(eval $(call gb_UnpackedTarball_set_post_action,skia,\
    mv third_party/skcms/skcms.cc third_party/skcms/skcms.cpp \
))

# vim: set noet sw=4 ts=4:
