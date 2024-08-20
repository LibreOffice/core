# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,freetype))

$(eval $(call gb_UnpackedTarball_set_tarball,freetype,$(FREETYPE_TARBALL),,freetype))

$(eval $(call gb_UnpackedTarball_add_patches,freetype,\
	external/freetype/freetype-2.6.5.patch.1 \
	external/freetype/ubsan.patch \
	external/freetype/freetype-fd-hack.patch.0 \
))

# Enable FreeType's FT_DEBUG_LOGGING at least in --enable-dbgutil builds (and see
# workdir/UnpackedTarball/freetype/docs/DEBUG for how to actually make use of that by setting an
# FT2_DEBUG environment variable at runtime):
ifeq ($(ENABLE_DBGUTIL),TRUE)
$(eval $(call gb_UnpackedTarball_add_patches,freetype, \
    external/freetype/logging.patch.0 \
))
endif

$(eval $(call gb_UnpackedTarball_set_patchlevel,freetype,0))

# vim: set noet sw=4 ts=4:
