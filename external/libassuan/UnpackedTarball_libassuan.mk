# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,libassuan))

$(eval $(call gb_UnpackedTarball_set_tarball,libassuan,$(LIBASSUAN_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,libassuan,0))

# 0001-const-up-std_cmd_table.patch.1
# upstream attempt made by git-email to gnupg-devel@gnupg.org

$(eval $(call gb_UnpackedTarball_add_patches,libassuan, \
    external/libassuan/find-libgpg-error.patch \
    external/libassuan/fix-autoconf-macros.patch \
    $(if $(filter MSC,$(COM)),external/libassuan/w32-build-fixes.patch.1) \
    external/libassuan/w32-build-fixes-2.patch \
    $(if $(filter LINUX,$(OS)),external/libassuan/rpath.patch) \
    external/libassuan/w32-stdc.patch \
    external/libassuan/0001-const-up-std_cmd_table.patch.1 \
))

# vim: set noet sw=4 ts=4:
