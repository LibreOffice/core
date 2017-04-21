# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,nss))

$(eval $(call gb_UnpackedTarball_set_tarball,nss,$(NSS_TARBALL)))

$(eval $(call gb_UnpackedTarball_add_patches,nss,\
	external/nss/nss.patch \
	external/nss/nss.aix.patch \
	external/nss/nss-3.13.5-zlib-werror.patch \
	$(if $(filter WNTMSC,$(OS)$(COM)),external/nss/nss.windows.patch) \
	$(if $(filter MSC-INTEL,$(COM)-$(CPUNAME)), \
		external/nss/nss.cygwin64.in32bit.patch) \
    $(if $(findstring 120_70,$(VCVER)_$(WINDOWS_SDK_VERSION)), \
        external/nss/nss-winXP-sdk.patch.1) \
	external/nss/nss-no-c99.patch \
))

# vim: set noet sw=4 ts=4:
