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
	lib/Makefile.vc12 \
))

$(eval $(call gb_UnpackedTarball_add_patches,curl,\
	external/curl/curl-msvc.patch.1 \
	external/curl/curl-msvc-schannel.patch.1 \
	external/curl/curl-7.26.0_win-proxy.patch \
	external/curl/curl-xp.patch.1 \
))

ifeq ($(SYSTEM_NSS),)
$(eval $(call gb_UnpackedTarball_add_patches,curl,\
	external/curl/curl-nss.patch.1 \
))
endif

# vim: set noet sw=4 ts=4:
