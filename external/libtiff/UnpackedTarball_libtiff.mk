# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,libtiff))

$(eval $(call gb_UnpackedTarball_set_tarball,libtiff,$(LIBTIFF_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,libtiff,1))

$(eval $(call gb_UnpackedTarball_add_patches,libtiff,\
    external/libtiff/libtiff.linknolibs.patch \
    external/libtiff/0001-ofz-54685-Timeout.patch \
    external/libtiff/ofz65182.patch \
    external/libtiff/ofz68840.patch \
))

# vim: set noet sw=4 ts=4:
