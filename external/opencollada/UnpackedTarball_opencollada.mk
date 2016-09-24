# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,opencollada))

$(eval $(call gb_UnpackedTarball_set_tarball,opencollada,$(OPENCOLLADA_TARBALL)))

$(eval $(call gb_UnpackedTarball_add_patches,opencollada,\
	external/opencollada/patches/0001-Revert-fix-for-malformed-FBX-COLLADA-id.patch.1 \
	external/opencollada/patches/0002-Expose-sid-for-textures-in-extras.patch.1 \
	external/opencollada/patches/opencollada.clang.patch.0 \
	external/opencollada/patches/opencollada.libxml.patch.0 \
	external/opencollada/patches/libc++.patch.0 \
))

# vim: set noet sw=4 ts=4:
