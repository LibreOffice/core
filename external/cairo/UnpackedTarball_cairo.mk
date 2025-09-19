# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,cairo))

$(eval $(call gb_UnpackedTarball_set_tarball,cairo,$(CAIRO_TARBALL),,cairo))

$(eval $(call gb_UnpackedTarball_set_patchlevel,cairo,2))

# The avoid-negative-left-shift patch, I'm not sure if it's the right thing. It triggers in JunitTest_chart2_unoapi.
#
# The ubsan-memcpy-nullptr is reported at https://gitlab.freedesktop.org/cairo/cairo/-/issues/884
#
$(eval $(call gb_UnpackedTarball_add_patches,cairo,\
    external/cairo/cairo/cairo.RGB24_888.patch \
    external/cairo/cairo/san.patch.0 \
    external/cairo/cairo/libcairo-bundled-soname.patch.1 \
    external/cairo/cairo/cairo-fd-hack.patch.0 \
    external/cairo/cairo/cairo.ofz57493-Timeout.patch.1 \
    external/cairo/cairo/shm-null-deref.patch.1 \
    external/cairo/cairo/avoid-negative-left-shift.patch.1 \
    external/cairo/cairo/ubsan-memcpy-nullptr.patch.1 \
    external/cairo/cairo/ubsan-memcpy-nullptr2.patch.1 \
    external/cairo/cairo/Wincompatible-pointer-types.patch.0 \
    external/cairo/cairo/disable-cairo-utilities.patch.1 \
))

ifeq ($(ENABLE_CAIRO_RGBA),TRUE)
$(eval $(call gb_UnpackedTarball_add_patches,cairo,\
    external/cairo/cairo/cairo.GL_RGBA.patch \
))
endif

ifneq (,$(filter MACOSX ANDROID iOS,$(OS)))
$(eval $(call gb_UnpackedTarball_add_file,cairo,.,external/cairo/cairo/dummy_pkg_config))
endif

# vim: set noet sw=4 ts=4:
