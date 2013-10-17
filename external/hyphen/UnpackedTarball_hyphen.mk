# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,hyphen))

$(eval $(call gb_UnpackedTarball_set_tarball,hyphen,$(HYPHEN_TARBALL)))

$(eval $(call gb_UnpackedTarball_add_patches,hyphen,\
	external/hyphen/hyphen-lenwaswrong.patch \
	external/hyphen/hyphen-fdo43931.patch \
	external/hyphen/hyphen-android.patch \
	external/hyphen/hyphen-rhmin.patch \
	external/hyphen/hyphen-build.patch \
))

# vim: set noet sw=4 ts=4:
