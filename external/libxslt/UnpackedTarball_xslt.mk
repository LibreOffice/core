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
	external/libxslt/libxslt-config.patch.1 \
	external/libxslt/libxslt-internal-symbols.patch \
	external/libxslt/libxslt-msvc.patch.2 \
	external/libxslt/libxslt-1.1.26-memdump.patch \
	external/libxslt/rpath.patch.0 \
	external/libxslt/e03553605b45c88f0b4b2980adfbbb8f6fca2fd6.patch.1 \
))

# vim: set noet sw=4 ts=4:
