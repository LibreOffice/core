# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,freetype))

$(eval $(call gb_UnpackedTarball_set_tarball,freetype,$(FREETYPE_TARBALL),,freetype))

$(eval $(call gb_UnpackedTarball_set_patchlevel,freetype,3))

$(eval $(call gb_UnpackedTarball_add_patches,freetype,\
	external/freetype/freetype-2.4.8.patch \
	external/freetype/freetype-msvc-disable-sse2.patch.1 \
))

# vim: set noet sw=4 ts=4:
