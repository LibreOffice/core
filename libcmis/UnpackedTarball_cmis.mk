# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,cmis))

$(eval $(call gb_UnpackedTarball_set_tarball,cmis,$(CMIS_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,cmis,0))

$(eval $(call gb_UnpackedTarball_add_patches,cmis, \
						libcmis/libcmis-0.4.1.patch \
						libcmis/libcmis-0.4.1-empty-path.patch))

ifeq ($(OS)$(COM),WNTMSC)
$(eval $(call gb_UnpackedTarball_add_patches,cmis,libcmis/boost-win.patch))
endif

# vim: set noet sw=4 ts=4:
