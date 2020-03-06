# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,gyp))

$(eval $(call gb_UnpackedTarball_set_tarball,gyp,$(GYP_TARBALL)))

$(eval $(call gb_UnpackedTarball_add_patches,gyp,\
	external/nss/gyp-buildsystem.patch.0 \
))

ifeq ($(OS),WNT)
$(eval $(call gb_UnpackedTarball_add_patches,gyp,\
	external/nss/gyp-buildsystem-windows.patch.0 \
))
endif

# vim: set noet sw=4 ts=4:
