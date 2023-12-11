# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,curl))

$(eval $(call gb_UnpackedTarball_set_tarball,curl,$(CURL_TARBALL),,curl))

$(eval $(call gb_UnpackedTarball_set_patchlevel,curl,1))

$(eval $(call gb_UnpackedTarball_fix_end_of_line,curl,\
	winbuild/MakefileBuild.vc \
))

$(eval $(call gb_UnpackedTarball_add_patches,curl,\
	external/curl/curl-msvc.patch.1 \
	external/curl/curl-msvc-disable-protocols.patch.1 \
	external/curl/curl-7.26.0_win-proxy.patch \
	external/curl/zlib.patch.0 \
	external/curl/CVE-2023-38545_7.87.0.patch \
	external/curl/2b0994c29a721c91c57.patch \
))

ifeq ($(SYSTEM_NSS),)
$(eval $(call gb_UnpackedTarball_add_patches,curl,\
	external/curl/curl-nss.patch.1 \
))
endif

ifeq ($(OS)-$(COM_IS_CLANG),WNT-TRUE)
$(eval $(call gb_UnpackedTarball_add_patches,curl, \
    external/curl/clang-cl.patch.0 \
))
endif

# vim: set noet sw=4 ts=4:
