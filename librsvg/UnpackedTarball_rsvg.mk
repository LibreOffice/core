# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,rsvg))

$(eval $(call gb_UnpackedTarball_set_tarball,rsvg,$(LIBRSVG_TARBALL)))

#$(eval $(call gb_UnpackedTarball_set_patchlevel,rsvg,3))

ifeq ($(OS),MACOSX)
$(eval $(call gb_UnpackedTarball_add_patches,rsvg,\
	librsvg/librsvg-2.32.1.patch \
))
else ifeq ($(OS),WNT)
$(eval $(call gb_UnpackedTarball_add_patches,rsvg,\
	librsvg/librsvg-2.32.1-win32.patch \
))
$(eval $(call gb_UnpackedTarball_add_files,rsvg,,\
	librsvg/config.h \
))
endif

# vim: set noet sw=4 ts=4: