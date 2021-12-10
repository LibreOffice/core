#-*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,zxing))

$(eval $(call gb_UnpackedTarball_set_tarball,zxing,$(ZXING_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,zxing,1))

$(eval $(call gb_UnpackedTarball_add_patches,zxing, \
	external/zxing/zxing_newline.patch.1 \
	external/zxing/0001-Use-a-patch-file-to-document-changes-from-upstream-s.patch \
	external/zxing/0002-Update-stb_image_write-from-1.14-to-1.16.patch \
	external/zxing/0003-Update-stb_image-from-2.25-to-2.27.patch \
	external/zxing/0004-Apply-stb-PR-1223-to-stb_image.patch \
))

# vim: set noet sw=4 ts=4:
