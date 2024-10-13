# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,libpng))

$(eval $(call gb_UnpackedTarball_set_tarball,libpng,$(LIBPNG_TARBALL)))

$(eval $(call gb_UnpackedTarball_add_files,libpng,.,\
	external/libpng/configs/pnglibconf.h \
))

$(eval $(call gb_UnpackedTarball_add_patches,libpng,\
	external/libpng/0001-ACES-AP0-adjusted-fixes.patch.1 \
	external/libpng/0001-libpng16-fix-Correct-the-function-png_fp_sub-in-png.patch.1 \
	external/libpng/0001-fix-Avoid-integer-overflows-in-function-png_xy_from_.patch.1 \
))

# vim: set noet sw=4 ts=4:
