# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,pixman))

$(eval $(call gb_UnpackedTarball_set_tarball,pixman,$(PIXMAN_TARBALL),,cairo))

$(eval $(call gb_UnpackedTarball_set_patchlevel,pixman,2))

$(eval $(call gb_UnpackedTarball_add_patches,pixman,\
	external/cairo/pixman/pixman-0.24.4.patch \
	external/cairo/pixman/pixman-ubsan.patch \
	external/cairo/pixman/pixman-wasm.patch \
))

# vim: set noet sw=4 ts=4:
