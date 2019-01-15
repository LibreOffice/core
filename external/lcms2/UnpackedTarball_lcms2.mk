# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,lcms2))

$(eval $(call gb_UnpackedTarball_set_tarball,lcms2,$(LCMS2_TARBALL)))

$(eval $(call gb_UnpackedTarball_update_autoconf_configs,lcms2))

$(eval $(call gb_UnpackedTarball_set_patchlevel,lcms2,3))

$(eval $(call gb_UnpackedTarball_add_patches,lcms2,\
	external/lcms2/0017-Upgrade-Visual-studio-2017-15.8.patch.1 \
	external/lcms2/lcms2-2.4-windows.patch \
	external/lcms2/lcms2-msvc-disable-sse2.patch.1 \
	external/lcms2/c++17.patch.0 \
))

# vim: set noet sw=4 ts=4:
