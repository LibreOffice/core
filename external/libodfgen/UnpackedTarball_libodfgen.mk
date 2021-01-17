# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,libodfgen))

$(eval $(call gb_UnpackedTarball_set_tarball,libodfgen,$(ODFGEN_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,libodfgen,0))

$(eval $(call gb_UnpackedTarball_update_autoconf_configs,libodfgen))

ifeq ($(SYSTEM_REVENGE),)
$(eval $(call gb_UnpackedTarball_add_patches,libodfgen, \
    external/libodfgen/rpath.patch \
))
endif

ifneq ($(OS),MACOSX)
ifneq ($(OS),WNT)
$(eval $(call gb_UnpackedTarball_add_patches,libodfgen, \
    external/libodfgen/libodfgen-bundled-soname.patch.0 \
))
endif
endif

# vim: set noet sw=4 ts=4:
