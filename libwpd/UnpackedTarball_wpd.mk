# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,wpd))

$(eval $(call gb_UnpackedTarball_set_tarball,wpd,$(WPD_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,wpd,0))

wpd_patches :=
# By mistake libwpd-0.9.5 changed ABI, change back
wpd_patches += libwpd-0.9.5-ABI.patch
# Link the static library against the dll runtime and not the static one
wpd_patches += libwpd-0.9.5-msc.patch

$(eval $(call gb_UnpackedTarball_add_patches,wpd,\
	$(foreach patch,$(wpd_patches),libwpd/$(patch)) \
))

# vim: set noet sw=4 ts=4:
