# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,libqxp))

$(eval $(call gb_UnpackedTarball_set_tarball,libqxp,$(QXP_TARBALL)))

$(eval $(call gb_UnpackedTarball_update_autoconf_configs,libqxp))

$(eval $(call gb_UnpackedTarball_set_patchlevel,libqxp,0))

$(eval $(call gb_UnpackedTarball_add_patches,libqxp, \
	external/libqxp/android-workaround.patch.1 \
	external/libcdr/ax_gcc_func_attribute.m4.patch \
))

# vim: set noet sw=4 ts=4:
