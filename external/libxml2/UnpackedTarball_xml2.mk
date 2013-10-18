# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,xml2))

$(eval $(call gb_UnpackedTarball_set_tarball,xml2,$(LIBXML_TARBALL),,libxml2))

$(eval $(call gb_UnpackedTarball_add_patches,xml2,\
	external/libxml2/libxml2-configure.patch \
	external/libxml2/libxml2-gnome599717.patch \
	external/libxml2/libxml2-xpath.patch \
	external/libxml2/libxml2-global-symbols.patch \
	external/libxml2/libxml2-aix.patch \
	external/libxml2/libxml2-vc10.patch \
	external/libxml2/libxml2-latin.patch \
	$(if $(filter ANDROID,$(OS)),external/libxml2/libxml2-android.patch) \
	$(if $(filter WNT,$(OS)),external/libxml2/libxml2-long-path.patch \
	$(if $(filter GCC,$(COM)),external/libxml2/libxml2-mingw.patch)) \
))

# vim: set noet sw=4 ts=4:
