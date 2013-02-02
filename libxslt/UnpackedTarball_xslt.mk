# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,xslt))

$(eval $(call gb_UnpackedTarball_set_tarball,xslt,$(LIBXSLT_TARBALL)))

$(eval $(call gb_UnpackedTarball_add_patches,xslt,\
	libxslt/libxslt-configure.patch \
	libxslt/libxslt-win_manifest.patch \
	libxslt/libxslt-mingw.patch \
	libxslt/libxslt-internal-symbols.patch \
	libxslt/libxslt-aix.patch \
	libxslt/libxslt-vc10.patch \
	libxslt/libxslt-1.1.26-memdump.patch \
	$(if $(filter ANDROID,$(OS)),libxslt/libxslt-android.patch) \
))

# vim: set noet sw=4 ts=4:
