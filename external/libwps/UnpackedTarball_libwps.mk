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

$(eval $(call gb_UnpackedTarball_set_patchlevel,libwps,1))

$(eval $(call gb_UnpackedTarball_update_autoconf_configs,libwps))

$(eval $(call gb_UnpackedTarball_add_patches,libwps,\
	$(if $(SYSTEM_REVENGE),,external/libwps/rpath.patch.0) \
))

ifneq ($(OS),MACOSX)
ifneq ($(OS),WNT)
$(eval $(call gb_UnpackedTarball_add_patches,libwps,\
	external/libwps/libwps-bundled-soname.patch.0 \
))
endif
endif

ifeq ($(COM_IS_CLANG),TRUE)
ifneq ($(filter -fsanitize=%,$(CC)),)
$(eval $(call gb_UnpackedTarball_add_patches,libwps, \
    external/libwps/ubsan-visibility.patch.0 \
))
endif
endif

# vim: set noet sw=4 ts=4:
