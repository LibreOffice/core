# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,visio))

$(eval $(call gb_UnpackedTarball_set_tarball,visio,$(VISIO_TARBALL)))

$(eval $(call gb_UnpackedTarball_add_patches,visio,\
    libvisio/libvisio-lineheight.patch \
))
# libvisio/libvisio-lineheight.patch: see upstream
#  <https://gerrit.libreoffice.org/gitweb?p=libvisio.git;a=commitdiff;
#  h=257e1896942a7028f1d0669ac6d4f61f294cb14d>
#  "Fixed line height is in inches :("

# vim: set noet sw=4 ts=4:
