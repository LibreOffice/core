#-*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,zxing))

$(eval $(call gb_UnpackedTarball_set_tarball,zxing,$(ZXING_TARBALL)))

ifneq ($(MSYSTEM),)
# the 2.3.0 tarball contains dangling symlinks (to a submodule component/experimental backend)
# git-bash/msys tar fails when extracting since MSYS defaults to not create those
ifeq ($(filter winsymlinks%,$(MSYS)),)
$(call gb_UnpackedTarball_get_target,zxing): export MSYS:=$(MSYS) winsymlinks
endif
endif

$(eval $(call gb_UnpackedTarball_add_patches,zxing, \
	external/zxing/0001-add-ZXVersion-h.patch \
))

# vim: set noet sw=4 ts=4:
