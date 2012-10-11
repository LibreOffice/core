# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,jpeg))

$(eval $(call gb_UnpackedTarball_set_tarball,jpeg,$(JPEG_TARBALL)))

$(eval $(call gb_UnpackedTarball_add_file,jpeg,jconfig.h,jpeg/configs/jconfig.h))

$(eval $(call gb_UnpackedTarball_add_patches,jpeg,\
    jpeg/patches/jpeg-8c-jmorecfg.patch \
))

ifeq ($(OS)$(CPU),MACOSXP)
$(eval $(call gb_UnpackedTarball_add_patches,jpeg,\
    jpeg/patches/struct_alignment.patch \
))
endif

# vim: set noet sw=4 ts=4:
