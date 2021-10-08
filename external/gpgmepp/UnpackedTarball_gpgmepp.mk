# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,gpgmepp))

$(eval $(call gb_UnpackedTarball_set_tarball,gpgmepp,$(GPGME_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,gpgmepp,0))

$(eval $(call gb_UnpackedTarball_add_patches,gpgmepp, \
    external/gpgmepp/find-libgpg-error-libassuan.patch \
    external/gpgmepp/fix-autoconf-macros.patch \
    external/gpgmepp/add-minimal-keyexport.patch \
    $(if $(filter MSC,$(COM)),external/gpgmepp/w32-build-fixes.patch.1) \
    $(if $(filter MSC,$(COM)),external/gpgmepp/w32-disable-docs.patch.1) \
    $(if $(filter MSC,$(COM)),external/gpgmepp/w32-fix-win32-macro.patch.1) \
    $(if $(filter MSC,$(COM)),external/gpgmepp/w32-fix-libtool.patch.1) \
    $(if $(filter MSC,$(COM)),external/gpgmepp/w32-add-initializer.patch.1) \
    external/gpgmepp/w32-build-fixes-2.patch \
    $(if $(filter LINUX,$(OS)),external/gpgmepp/asan.patch) \
    $(if $(filter LINUX,$(OS)),external/gpgmepp/rpath.patch) \
    external/gpgmepp/gpgme.git-4b64774b6d13ffa4f59dddf947a97d61bcfa2f2e.patch.1 \
))
# vim: set noet sw=4 ts=4:
