# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

libeot_patches :=
libeot_patches += 0001-remove-unused-err.h.patch
libeot_patches += debug-no-werror.patch.0

$(eval $(call gb_UnpackedTarball_UnpackedTarball,libeot))

$(eval $(call gb_UnpackedTarball_set_tarball,libeot,$(LIBEOT_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,libeot,1))

$(eval $(call gb_UnpackedTarball_add_patches,libeot,\
	$(foreach patch,$(libeot_patches),external/libeot/$(patch)) \
))

# vim: set noet sw=4 ts=4:
