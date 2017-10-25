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

$(eval $(call gb_UnpackedTarball_add_patches,libcmis, \
						external/libcmis/libcmis-libxml2_compatibility.patch \
						external/libcmis/libcmis-fix-google-drive.patch \
						external/libcmis/libcmis-google-2FA-implementation.patch \
						external/libcmis/libcmis-fix-google-drive-2.patch \
						external/libcmis/libcmis-sharepoint-repository-root.patch \
						external/libcmis/libcmis-fix-error-handling.patch \
						external/libcmis/c++17.patch.0 \
))

ifeq ($(OS),WNT)
$(eval $(call gb_UnpackedTarball_add_patches,libcmis,external/libcmis/boost-win.patch))
endif

# vim: set noet sw=4 ts=4:
