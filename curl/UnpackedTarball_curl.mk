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
	Makefile.msvc.names \
	lib/Makefile.vc9 \
))
$(eval $(call gb_UnpackedTarball_add_patches,curl,\
	curl/curl-7.26.0.patch \
	curl/curl-7.26.0_nspr.patch \
	curl/curl-aix.patch \
	curl/curl-7.26.0_win.patch \
	curl/curl-7.26.0_mingw.patch \
	curl/curl-7.26.0_win-proxy.patch \
))
ifeq ($(OS),ANDROID)
$(eval $(call gb_UnpackedTarball_add_patches,curl,\
	curl/curl-android.patch \
))
endif
# vim: set noet sw=4 ts=4:
