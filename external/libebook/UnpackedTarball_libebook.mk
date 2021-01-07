# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,libebook))

$(eval $(call gb_UnpackedTarball_set_tarball,libebook,$(EBOOK_TARBALL)))

$(eval $(call gb_UnpackedTarball_update_autoconf_configs,libebook))

$(eval $(call gb_UnpackedTarball_set_patchlevel,libebook,0))

$(eval $(call gb_UnpackedTarball_add_patches,libebook, \
    external/libebook/libebook-no-icu-boolean.patch.1 \
    external/libebook/include.patch \
))

# vim: set noet sw=4 ts=4:
