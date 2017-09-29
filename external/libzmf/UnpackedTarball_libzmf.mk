# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,libzmf))

$(eval $(call gb_UnpackedTarball_set_tarball,libzmf,$(ZMF_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,libzmf,0))

$(eval $(call gb_UnpackedTarball_update_autoconf_configs,libzmf))

ifeq ($(COM_IS_CLANG),TRUE)
ifneq ($(filter -fsanitize=%,$(CC)),)
$(eval $(call gb_UnpackedTarball_add_patches,libzmf, \
    external/libzmf/ubsan-visibility.patch \
))
endif
endif

$(eval $(call gb_UnpackedTarball_add_patches,libzmf, \
    external/libzmf/android-workaround.patch.1 \
))

# vim: set noet sw=4 ts=4:
