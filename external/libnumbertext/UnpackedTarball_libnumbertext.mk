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

$(eval $(call gb_UnpackedTarball_set_patchlevel,libnumbertext,1))

#TODO: external/libnumbertext/Afl++ICE.patch1 works around
# <https://github.com/AFLplusplus/AFLplusplus/commit/333509bb0a56be9bd2e236f0e2f37d4af2dd7d59>
# "better unicode support" for now:
$(eval $(call gb_UnpackedTarball_add_patches,libnumbertext, \
    external/libnumbertext/MSVCNonBMPBug.patch1 \
    external/libnumbertext/WinUnicodePath.patch1 \
    external/libnumbertext/Afl++ICE.patch1 \
))

# vim: set noet sw=4 ts=4:
