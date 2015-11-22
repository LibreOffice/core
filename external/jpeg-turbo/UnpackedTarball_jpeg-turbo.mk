# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,jpeg-turbo))

$(eval $(call gb_UnpackedTarball_set_tarball,jpeg-turbo,$(JPEG_TURBO_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,jpeg-turbo,0))

$(eval $(call gb_UnpackedTarball_add_patches,jpeg-turbo,\
	external/jpeg-turbo/jpeg-turbo.build.patch.1 \
	$(if $(filter WNT,$(OS)),external/jpeg-turbo/jpeg-turbo.win_build.patch.1) \
))

# vim: set noet sw=4 ts=4:
