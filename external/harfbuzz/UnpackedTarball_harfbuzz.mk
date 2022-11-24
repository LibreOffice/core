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

#   <https://github.com/harfbuzz/harfbuzz/pull/3874> "Adjust unsafe-to-break for non-zero ValueFormat2":
$(eval $(call gb_UnpackedTarball_add_patches,harfbuzz, \
    external/harfbuzz/3874.patch.1 \
    external/harfbuzz/0001-hb-view-Fix-cairo-slanting-condition.patch.1 \
    external/harfbuzz/0001-font-Respect-subfont-slant-setting-in-hb-draw.patch.1 \
    external/harfbuzz/0001-buffer-diff-Fix-check-for-glyph-flag-equality.patch.1 \
    external/harfbuzz/0001-PairPos-Another-attempt-at-fixing-unsafe-to-break-wi.patch.1 \
))

# vim: set noet sw=4 ts=4:
