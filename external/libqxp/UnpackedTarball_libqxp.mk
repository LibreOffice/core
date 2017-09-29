# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,libqxp))

$(eval $(call gb_UnpackedTarball_set_tarball,libqxp,$(QXP_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,libqxp,0))

$(eval $(call gb_UnpackedTarball_update_autoconf_configs,libqxp))

ifeq ($(COM_IS_CLANG),TRUE)
ifneq ($(filter -fsanitize=%,$(CC)),)
$(eval $(call gb_UnpackedTarball_add_patches,libqxp, \
    external/libqxp/ubsan-visibility.patch \
))
endif
endif

$(eval $(call gb_UnpackedTarball_add_patches,libqxp, \
	external/libqxp/0001-don-t-use-RVNGStringStream-in-the-main-lib.patch.1 \
	external/libqxp/0001-add-missing-includes.patch.1 \
	external/libqxp/0001-WaE-narrowing-conversion.patch.1 \
	external/libqxp/android-workaround.patch.1 \
))

# vim: set noet sw=4 ts=4:
