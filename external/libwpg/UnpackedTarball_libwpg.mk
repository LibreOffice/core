# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,libwpg))

$(eval $(call gb_UnpackedTarball_set_tarball,libwpg,$(WPG_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,libwpg,0))

$(eval $(call gb_UnpackedTarball_update_autoconf_configs,libwpg))

# 0001-const-up-mimeTypesMap.patch.1
# upstream attempt at:
# https://sourceforge.net/p/libwpg/code/merge-requests/1/

$(eval $(call gb_UnpackedTarball_add_patches,libwpg, \
    external/libwpg/rpath.patch \
    external/libwpg/0001-const-up-mimeTypesMap.patch.1 \
))

ifneq ($(OS),MACOSX)
ifneq ($(OS),WNT)
ifneq ($(OS),iOS)
$(eval $(call gb_UnpackedTarball_add_patches,libwpg, \
    external/libwpg/libwpg-bundled-soname.patch.0 \
))
endif
endif
endif

# vim: set noet sw=4 ts=4:
