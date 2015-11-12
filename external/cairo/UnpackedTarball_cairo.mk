# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,cairo))

$(eval $(call gb_UnpackedTarball_set_tarball,cairo,$(CAIRO_TARBALL),,cairo))

$(eval $(call gb_UnpackedTarball_add_patches,cairo,\
	external/cairo/cairo/cairo-1.10.2.patch \
	external/cairo/cairo/cairo.dlsym.lcdfilter.patch \
	external/cairo/cairo/cairo-1.10.2-oldfontconfig.patch \
))

ifeq ($(OS)$(COM),WNTMSC)
$(eval $(call gb_UnpackedTarball_add_patches,cairo,\
	external/cairo/cairo/cairo-1.10.2.wntmsc.patch \
))
endif

# FIXME add cairo/cairo/cairo-1.10.2.no-atsui.patch for MACOSX >= 1070
ifeq ($(OS),IOS)
$(eval $(call gb_UnpackedTarball_add_patches,cairo,\
	external/cairo/cairo/cairo-1.10.2.no-atsui.patch \
	external/cairo/cairo/cairo-1.10.2.ios.patch \
))
endif

ifeq ($(OS),ANDROID)
$(eval $(call gb_UnpackedTarball_add_patches,cairo,\
	external/cairo/cairo/cairo-1.10.2.android.patch \
))
endif

ifneq (,$(filter ANDROID IOS,$(OS)))
$(eval $(call gb_UnpackedTarball_add_file,cairo,.,external/cairo/cairo/dummy_pkg_config))
endif

ifeq ($(COM_IS_CLANG),TRUE)
$(eval $(call gb_UnpackedTarball_add_patches,cairo,\
	external/cairo/cairo/no-flto-clang.patch \
))
endif

# vim: set noet sw=4 ts=4:
