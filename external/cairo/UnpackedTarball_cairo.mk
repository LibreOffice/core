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
	external/cairo/cairo/cairo-libtool-rpath.patch.1 \
<<<<<<< HEAD   (2a65be Fix #118767 - Identify Firebird DBMS in DFunction())
=======
	external/cairo/cairo/cairo.oldfreetype.patch \
	external/cairo/cairo/san.patch.0 \
	external/cairo/cairo/0001-Fix-mask-usage-in-image-compositor.patch.1 \
>>>>>>> CHANGE (c767e9 external/cairo: Fix mask usage in image-compositor)
))

ifeq ($(OS),iOS)
$(eval $(call gb_UnpackedTarball_add_patches,cairo,\
	external/cairo/cairo/cairo-1.10.2.no-atsui.patch \
	external/cairo/cairo/cairo-1.10.2.ios.patch \
))
endif

# To be applied only when ENABLE_ANDROID_LOK is not defined
ifeq ($(OS)$(ENABLE_ANDROID_LOK),ANDROID)
$(eval $(call gb_UnpackedTarball_add_patches,cairo,\
	external/cairo/cairo/cairo.GL_RGBA.patch \
))
endif

ifneq (,$(filter ANDROID iOS,$(OS)))
$(eval $(call gb_UnpackedTarball_add_file,cairo,.,external/cairo/cairo/dummy_pkg_config))
endif

# vim: set noet sw=4 ts=4:
