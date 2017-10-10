# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,libjpeg-turbo))

$(eval $(call gb_UnpackedTarball_set_tarball,libjpeg-turbo,$(LIBJPEG_TURBO_TARBALL)))

$(eval $(call gb_UnpackedTarball_update_autoconf_configs,libjpeg-turbo))

$(eval $(call gb_UnpackedTarball_set_patchlevel,libjpeg-turbo,0))

$(eval $(call gb_UnpackedTarball_add_patches,libjpeg-turbo,\
	external/libjpeg-turbo/jpeg-turbo.build.patch.1 \
	$(if $(filter WNT,$(OS)),external/libjpeg-turbo/jpeg-turbo.win_build.patch.1) \
	external/libjpeg-turbo/ubsan.patch \
))

# vim: set noet sw=4 ts=4:
