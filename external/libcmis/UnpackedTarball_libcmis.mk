# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,libcmis))

$(eval $(call gb_UnpackedTarball_set_tarball,libcmis,$(LIBCMIS_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,libcmis,1))

$(eval $(call gb_UnpackedTarball_add_patches,libcmis,\
	external/libcmis/0001-fix-regression-in-HttpSession-initProtocols.patch \
	external/libcmis/0002-HttpSession-add-a-callback-that-can-be-used-to-confi.patch \
	external/libcmis/0003-HttpSession-fix-regression-setting-wrong-type-of-CUR.patch \
))

# vim: set noet sw=4 ts=4:
