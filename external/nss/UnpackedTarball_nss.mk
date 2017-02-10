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
	external/nss/nss_macosx.patch \
	external/nss/nss-win32-make.patch.1 \
	$(if $(filter WNT,$(OS)),external/nss/nss.windows.patch \
        external/nss/nss.nowerror.patch \
		external/nss/nss.vs2015.patch) \
    external/nss/ubsan.patch.0 \
    external/nss/clang-cl.patch.0 \
    external/nss/nss.windowbuild.patch.0 \
    $(if $(filter IOS,$(OS)), \
        external/nss/nss-chromium-nss-static.patch \
        external/nss/nss-more-static.patch \
        external/nss/nss-ios.patch) \
	$(if $(filter MSC-INTEL,$(COM)-$(CPUNAME)), \
		external/nss/nss.cygwin64.in32bit.patch) \
	$(if $(filter WNT,$(OS)), \
		external/nss/nss.vs2015.pdb.patch) \
    $(if $(findstring 120_70,$(VCVER)_$(WINDOWS_SDK_VERSION)), \
        external/nss/nss-winXP-sdk.patch.1) \
	$(if $(filter WNT,$(OS)), \
    	external/nss/nss.utf8bom.patch.1) \
))

ifeq ($(COM_IS_CLANG),TRUE)
ifneq ($(filter -fsanitize=%,$(CC)),)
$(eval $(call gb_UnpackedTarball_add_patches,nss,\
	external/nss/asan.patch.1 \
	external/nss/ubsan-alignment.patch.0 \
))
endif
endif

# vim: set noet sw=4 ts=4:
