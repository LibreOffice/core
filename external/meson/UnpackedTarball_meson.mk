# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,meson))

$(eval $(call gb_UnpackedTarball_set_tarball,meson,$(MESON_TARBALL)))

ifeq ($(OS),WNT)
# hack to make meson use a specific VS version and not always the latest
# (argument is '-latest' so inner quotes to separate the arguments
# it also always specifies -prerelease, but that is left alone here)
ifneq ($(filter 17.%,$(VCVER)),)
meson_vsversion=-version', '[17,18)
else ifneq ($(filter 16.%,$(VCVER)),)
meson_vsversion=-version', '[16,17)
else
$(warning Warning: Unhandled VCVER '$(VCVER)'; defaulting meson to use latest VS version)
meson_vsversion=-latest
endif

$(eval $(call gb_UnpackedTarball_set_post_action,meson,\
    sed -i -e "s/-latest/$(meson_vsversion)/" mesonbuild/utils/vsenv.py \
))
endif
# vim: set noet sw=4 ts=4:
