# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,libstaroffice))

$(eval $(call gb_UnpackedTarball_set_tarball,libstaroffice,$(STAROFFICE_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,libstaroffice,0))

$(eval $(call gb_UnpackedTarball_update_autoconf_configs,libstaroffice))

# 0001-const-up-these-arrays.patch.1
# upstream attempt as:
# https://github.com/fosnola/libstaroffice/pull/12

$(eval $(call gb_UnpackedTarball_add_patches,libstaroffice, \
    external/libstaroffice/0001-const-up-these-arrays.patch.1 \
))

ifneq ($(OS),MACOSX)
ifneq ($(OS),WNT)
$(eval $(call gb_UnpackedTarball_add_patches,libstaroffice, \
    external/libstaroffice/libstaroffice-bundled-soname.patch.0 \
))
endif
endif

ifeq ($(SYSTEM_REVENGE),)
$(eval $(call gb_UnpackedTarball_add_patches,libstaroffice, \
    external/libstaroffice/rpath.patch \
))
endif

# vim: set noet sw=4 ts=4:
