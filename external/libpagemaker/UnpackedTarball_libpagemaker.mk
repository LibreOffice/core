# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,libpagemaker))

$(eval $(call gb_UnpackedTarball_set_tarball,libpagemaker,$(PAGEMAKER_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,libpagemaker,1))

$(eval $(call gb_UnpackedTarball_add_patches,libpagemaker,\
	external/libpagemaker/0001-define-M_PI-if-not-available.patch \
	external/libpagemaker/0002-add-missing-typedef-on-windows.patch \
	external/libpagemaker/0003-round-is-not-available-on-windows.patch \
))

# vim: set noet sw=4 ts=4:
