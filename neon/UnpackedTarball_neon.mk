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
	neon/configs/config.h \
))

$(eval $(call gb_UnpackedTarball_set_patchlevel,neon,0))

$(eval $(call gb_UnpackedTarball_add_patches,neon,\
	neon/neon.patch \
	neon/neon_ne_set_request_flag.patch \
))

# vim: set noet sw=4 ts=4:
