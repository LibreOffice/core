# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,md4c))
$(eval $(call gb_UnpackedTarball_set_tarball,md4c,$(MD4C_TARBALL)))

# 0001-const-up-scheme_map.patch.1 effort to upstream at:
# https://github.com/mity/md4c/pull/291

$(eval $(call gb_UnpackedTarball_add_patches,md4c,\
	external/md4c/0001-const-up-scheme_map.patch.1 \
))


# vim: set noet sw=4 ts=4:
