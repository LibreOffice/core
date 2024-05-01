#-*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,dtoa))

$(eval $(call gb_UnpackedTarball_set_tarball,dtoa,$(DTOA_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,dtoa,1))

$(eval $(call gb_UnpackedTarball_add_patches,dtoa, \
    external/dtoa/include_header.patch \
    external/dtoa/coverity.patch \
    external/dtoa/ubsan.patch.0 \
    external/dtoa/C6011.patch.0 \
))

# vim: set noet sw=4 ts=4:
