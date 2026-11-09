# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,libnumbertext))

$(eval $(call gb_UnpackedTarball_set_tarball,libnumbertext,$(LIBNUMBERTEXT_TARBALL)))

$(eval $(call gb_UnpackedTarball_update_autoconf_configs,libnumbertext))

$(eval $(call gb_UnpackedTarball_add_patches,libnumbertext, \
    external/libnumbertext/MSVCNonBMPBug.patch \
    external/libnumbertext/WinUnicodePath.patch \
    external/libnumbertext/deprecated.patch.0 \
    external/libnumbertext/libnumbertext-call-limit.patch.1 \
    external/libnumbertext/libnumbertext-depth-abort.patch.1 \
    external/libnumbertext/libnumbertext-input-check.patch.1 \
    external/libnumbertext/libnumbertext-catch-exceptions.patch.1 \
    external/libnumbertext/libnumbertext-module-cache.patch.1 \
))

# the libnumbertext-*.patch.1 patches are in https://github.com/Numbertext/libnumbertext/pull/142

# vim: set noet sw=4 ts=4:
