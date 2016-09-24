# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,glyphy))

$(eval $(call gb_UnpackedTarball_set_tarball,glyphy,$(GLYPHY_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,glyphy,1))

$(eval $(call gb_UnpackedTarball_add_patches,glyphy,\
	external/glyphy/glyphy-upstream.patch.1 \
	external/glyphy/glyphy-windows.patch.1 \
))

# vim: set noet sw=4 ts=4:
