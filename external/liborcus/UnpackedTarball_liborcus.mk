# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,liborcus))

$(eval $(call gb_UnpackedTarball_set_tarball,liborcus,$(ORCUS_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,liborcus,1))

$(eval $(call gb_UnpackedTarball_update_autoconf_configs,liborcus))

# external/liborcus/0001-Blind-fix-attempt-for-older-macOS-builds.patch upstreamed as
#  <https://gitlab.com/orcus/orcus/commit/a124e3b46f5430fa1c13c5500f3cad6ed147f068> "Blind fix
#  attempt for older macOS builds":
$(eval $(call gb_UnpackedTarball_add_patches,liborcus,\
	external/liborcus/rpath.patch.0 \
	external/liborcus/gcc9.patch.0 \
	external/liborcus/libtool.patch.0 \
	external/liborcus/0001-Blind-fix-attempt-for-older-macOS-builds.patch \
))

ifeq ($(OS),WNT)
$(eval $(call gb_UnpackedTarball_add_patches,liborcus,\
	external/liborcus/windows-constants-hack.patch \
))
endif

# vim: set noet sw=4 ts=4:
