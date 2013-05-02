# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,odfgen))

$(eval $(call gb_UnpackedTarball_set_tarball,odfgen,$(ODFGEN_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,odfgen,1))

$(eval $(call gb_UnpackedTarball_add_patches,odfgen,\
       libodfgen/libodfgen-0.0.0-android.patch \
))

# vim: set noet sw=4 ts=4:
