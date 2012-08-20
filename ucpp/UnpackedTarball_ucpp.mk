# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,ucpp))

$(eval $(call gb_UnpackedTarball_set_tarball,ucpp,0168229624cfac409e766913506961a8-ucpp-1.3.2.tar.gz))

$(eval $(call gb_UnpackedTarball_set_patchlevel,ucpp,0))

$(eval $(call gb_UnpackedTarball_add_patches,ucpp,\
    ucpp/ucpp.patch \
))

# vim: set noet sw=4 ts=4:
