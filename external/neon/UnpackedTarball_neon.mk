# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,neon))

$(eval $(call gb_UnpackedTarball_set_tarball,neon,$(NEON_TARBALL)))

$(eval $(call gb_UnpackedTarball_add_files,neon,src,\
	external/neon/configs/config.h \
))

$(eval $(call gb_UnpackedTarball_set_patchlevel,neon,0))

ifeq ($(OS),WNT)
$(eval $(call gb_UnpackedTarball_add_patches,neon,\
	external/neon/neon.patch \
	external/neon/neon_ne_set_request_flag.patch \
	external/neon/neon_with_gnutls.patch \
	external/neon/ubsan.patch \
	external/neon/neon_fix_lock_token_on_if.patch \
	external/neon/neon_fix_lock_timeout_windows.patch \
))
else
$(eval $(call gb_UnpackedTarball_add_patches,neon,\
	external/neon/neon.patch \
	external/neon/neon_ne_set_request_flag.patch \
	external/neon/neon_with_gnutls.patch \
	external/neon/ubsan.patch \
	external/neon/neon_fix_lock_token_on_if.patch \
))
endif

# vim: set noet sw=4 ts=4:
