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

# forcepoint-83.patch.1 merged as
# https://gitlab.com/orcus/orcus/-/commit/9f6400b8192e39fefd475a96222713e9e9c60038
# forcepoint-84.patch.1 merged as
# https://gitlab.com/orcus/orcus/-/commit/223defe95d6f20f1bc5fd22fecc80a79a9519028
# forcepoint-87.patch.1 merged as
# https://gitlab.com/orcus/orcus/-/commit/a718524ca424fb8a7e7931345a118342d1d4a507
# forcepoint-88.patch.1 merged as
# https://gitlab.com/orcus/orcus/-/commit/0a99ca6d50af51f1b0a151fdcac5e12ec9b01bf8
# forcepoint-95.patch.1 submitted as
# https://gitlab.com/orcus/orcus/-/merge_requests/124
# overrun.patch.0 submitted as
# https://gitlab.com/orcus/orcus/-/merge_requests/131

$(eval $(call gb_UnpackedTarball_add_patches,liborcus,\
	external/liborcus/rpath.patch.0 \
	external/liborcus/gcc9.patch.0 \
	external/liborcus/libtool.patch.0 \
	external/liborcus/fix-pch.patch.0 \
	external/liborcus/liborcus_newline.patch.1 \
	external/liborcus/std-get-busted.patch.1 \
	external/liborcus/forcepoint-83.patch.1 \
	external/liborcus/forcepoint-84.patch.1 \
	external/liborcus/forcepoint-87.patch.1 \
	external/liborcus/forcepoint-88.patch.1 \
	external/liborcus/forcepoint-95.patch.1 \
	external/liborcus/include.patch.0 \
	external/liborcus/overrun.patch.0 \
))

ifeq ($(OS),WNT)
$(eval $(call gb_UnpackedTarball_add_patches,liborcus,\
	external/liborcus/windows-constants-hack.patch \
	external/liborcus/win_path_utf16.patch \
))
endif

# vim: set noet sw=4 ts=4:
