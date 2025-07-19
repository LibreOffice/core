# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,libwps))

$(eval $(call gb_UnpackedTarball_set_tarball,libwps,$(WPS_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,libwps,0))

$(eval $(call gb_UnpackedTarball_update_autoconf_configs,libwps))

# external/libwps/0001-const-up-some-things.patch.1
# upstream effort as: https://sourceforge.net/p/libwps/code/merge-requests/1/

$(eval $(call gb_UnpackedTarball_add_patches,libwps,\
	$(if $(SYSTEM_REVENGE),,external/libwps/rpath.patch.0) \
	external/libwps/libtool.patch.0 \
	external/libwps/enumarith.patch \
	external/libwps/0001-const-up-some-things.patch.1 \
))

ifneq ($(OS),MACOSX)
ifneq ($(OS),WNT)
$(eval $(call gb_UnpackedTarball_add_patches,libwps,\
	external/libwps/libwps-bundled-soname.patch.0 \
))
endif
endif

# vim: set noet sw=4 ts=4:
