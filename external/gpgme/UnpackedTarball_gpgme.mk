# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,gpgme))

$(eval $(call gb_UnpackedTarball_set_tarball,gpgme,$(GPGME_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,gpgme,0))

$(eval $(call gb_UnpackedTarball_add_patches,gpgme, \
    external/gpgme/find-libgpg-error-libassuan.patch \
    external/gpgme/fix-autoconf-macros.patch \
    $(if $(filter MSC,$(COM)),external/gpgme/w32-build-fixes.patch.1) \
))
# vim: set noet sw=4 ts=4:
