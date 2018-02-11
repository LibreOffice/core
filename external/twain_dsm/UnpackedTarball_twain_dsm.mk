# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,twain_dsm))

$(eval $(call gb_UnpackedTarball_set_tarball,twain_dsm,$(TWAIN_DSM_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,twain_dsm,1))

$(eval $(call gb_UnpackedTarball_add_patches,twain_dsm, \
    external/twain_dsm/TWAIN_DSM_VS2015.vcxproj.patch \
    external/twain_dsm/TWAIN_DSM_VS2015.vcxproj.filters.patch \
    external/twain_dsm/fix-non-us-ascii-chars-part1.patch \
    external/twain_dsm/fix-non-us-ascii-chars-part2.patch \
))

# vim: set noet sw=4 ts=4:
