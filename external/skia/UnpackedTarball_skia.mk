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

# TODO
skia_patches := \
    lerp.patch \
    fix-pch.patch \
    fix-ddi.patch \
    make-api-visible.patch.1 \
    fix-shader-locale.patch.1 \
    no-trace-resources-on-exit.patch.1 \
    share-grcontext.patch.1

$(eval $(call gb_UnpackedTarball_set_patchlevel,skia,1))

$(eval $(call gb_UnpackedTarball_add_patches,skia,\
    $(foreach patch,$(skia_patches),external/skia/$(patch)) \
))

$(eval $(call gb_UnpackedTarball_set_post_action,skia,\
    mv third_party/skcms/skcms.cc third_party/skcms/skcms.cpp \
))

# vim: set noet sw=4 ts=4:
