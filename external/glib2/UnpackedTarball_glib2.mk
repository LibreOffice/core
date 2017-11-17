# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,glib2))

$(eval $(call gb_UnpackedTarball_set_tarball,glib2,$(GLIB2_TARBALL)))

# $(eval $(call gb_UnpackedTarball_add_patches,glib2,\
# 	external/glib2/glib-2.28.1.noise.patch \
# 	external/glib2/glib-2.28.1.patch \
# 	external/glib2/glib-2.28.1-win32-2.patch \
# 	external/glib2/glib-2.28.1-win32.patch \
))

# vim: set noet sw=4 ts=4:
