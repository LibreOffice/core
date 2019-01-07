# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,libetonyek))

$(eval $(call gb_UnpackedTarball_set_tarball,libetonyek,$(ETONYEK_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,libetonyek,0))

$(eval $(call gb_UnpackedTarball_update_autoconf_configs,libetonyek))

$(eval $(call gb_UnpackedTarball_add_patches,libetonyek,\
	external/libetonyek/win_build.patch.1 \
	external/libetonyek/ubsan.patch \
	external/libetonyek/rpath.patch \
	external/libetonyek/warnings.patch \
))

ifneq ($(OS),MACOSX)
ifneq ($(OS),WNT)
ifneq ($(OS),iOS)
$(eval $(call gb_UnpackedTarball_add_patches,libetonyek,\
	external/libetonyek/libetonyek-bundled-soname.patch.0 \
))
endif
endif
endif

ifeq ($(COM_IS_CLANG),TRUE)
ifneq ($(filter -fsanitize=%,$(CC)),)
$(eval $(call gb_UnpackedTarball_add_patches,libetonyek, \
    external/libetonyek/ubsan-visibility.patch \
))
endif
endif

# vim: set noet sw=4 ts=4:
