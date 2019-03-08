# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,harfbuzz))

$(eval $(call gb_UnpackedTarball_set_tarball,harfbuzz,$(HARFBUZZ_TARBALL),,harfbuzz))

$(eval $(call gb_UnpackedTarball_update_autoconf_configs,harfbuzz))

$(eval $(call gb_UnpackedTarball_set_patchlevel,harfbuzz,0))

# * external/harfbuzz/msvc.patch sent upstream as <https://github.com/harfbuzz/harfbuzz/pull/1605>
#   "Fix hb_atomic_* variants based on C++11 atomics":
$(eval $(call gb_UnpackedTarball_add_patches,harfbuzz, \
    external/harfbuzz/msvc.patch \
))

# vim: set noet sw=4 ts=4:
