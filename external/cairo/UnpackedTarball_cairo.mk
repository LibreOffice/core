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

$(eval $(call gb_UnpackedTarball_set_patchlevel,cairo,2))

# cairo >= 1.17.6 was probably created in Fedora where
# https://salsa.debian.org/mckinstry/libtool/-/commit/26c23f951d049241128e5e04a7bbc263e5b145f1
# isn't applied, so add that in to avoid: /usr/bin/ld: unrecognized option '--gdb-index'

# ofz50805.patch https://gitlab.freedesktop.org/cairo/cairo/-/merge_requests/350
# ofz46165.patch upstreamed as https://gitlab.freedesktop.org/cairo/cairo/-/merge_requests/351
# san.patch upstreamed as https://gitlab.freedesktop.org/cairo/cairo/-/merge_requests/352

$(eval $(call gb_UnpackedTarball_add_patches,cairo,\
	external/cairo/cairo/cairo.buildfix.patch \
	external/cairo/cairo/cairo.RGB24_888.patch \
	external/cairo/cairo/cairo-libtool-rpath.patch.1 \
	external/cairo/cairo/cairo.oldfreetype.patch \
	external/cairo/cairo/san.patch.0 \
	external/cairo/cairo/cairo.ofz46165.patch.1 \
	external/cairo/cairo/cairo.ofz50805.patch.1 \
	external/cairo/cairo/0025-libtool-pass-use-ld.patch \
	external/cairo/cairo/libcairo-bundled-soname.patch.0 \
	external/cairo/cairo/cairo-fd-hack.patch.0 \
	external/cairo/cairo/cairo.ofz57493-Timeout.patch.1 \
))

ifeq ($(OS),iOS)
$(eval $(call gb_UnpackedTarball_add_patches,cairo,\
	external/cairo/cairo/cairo-1.10.2.no-atsui.patch \
	external/cairo/cairo/cairo-1.10.2.ios.patch \
))
endif

ifeq ($(ENABLE_CAIRO_RGBA),TRUE)
$(eval $(call gb_UnpackedTarball_add_patches,cairo,\
	external/cairo/cairo/cairo.GL_RGBA.patch \
))
endif

ifneq (,$(filter ANDROID iOS,$(OS)))
$(eval $(call gb_UnpackedTarball_add_file,cairo,.,external/cairo/cairo/dummy_pkg_config))
endif

# vim: set noet sw=4 ts=4:
