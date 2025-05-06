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

# See https://github.com/harfbuzz/harfbuzz/pull/5315 for
# 0001-increase-maximum-set_flags-range.patch.1

$(eval $(call gb_UnpackedTarball_add_patches,harfbuzz, \
    external/harfbuzz/tdf159529.patch.0 \
    external/harfbuzz/harfbuzz_visibility.patch.1 \
    external/harfbuzz/0001-increase-maximum-set_flags-range.patch.1 \
))

# meson will replace python3 from shebang in build commands with the
# interpreter it is run with (which is what we want for python=fully-internal
# case to avoid incompatibilities with the system python version).
# And while it does so unconditionally on windows, on other platforms that only
# happens if the command to run doesn't have the executable bit set.
$(eval $(call gb_UnpackedTarball_set_post_action,harfbuzz,chmod a-x src/*.py))

# vim: set noet sw=4 ts=4:
