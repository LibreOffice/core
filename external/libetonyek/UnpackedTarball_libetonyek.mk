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

$(eval $(call gb_UnpackedTarball_add_patches,libetonyek,\
	external/libetonyek/assert.patch \
	external/libetonyek/0001-missing-include.patch.1 \
	external/libetonyek/win_build.patch.1 \
	external/libetonyek/ubsan.patch \
	external/libetonyek/0001-add-missing-breaks.patch.1 \
	external/libetonyek/0002-remove-surplus-breaks.patch.1 \
	external/libetonyek/rpath.patch \
))

ifeq ($(COM_GCC_IS_CLANG),TRUE)
ifneq ($(filter -fsanitize=%,$(CC)),)
$(eval $(call gb_UnpackedTarball_add_patches,libetonyek, \
    external/libetonyek/ubsan-visibility.patch \
))
endif
endif

# vim: set noet sw=4 ts=4:
