# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,jfreereport_flute))

$(eval $(call gb_UnpackedTarball_set_tarball,jfreereport_flute,$(JFREEREPORT_FLUTE_TARBALL),0))

$(eval $(call gb_UnpackedTarball_fix_end_of_line,jfreereport_flute,\
	common_build.xml \
))

$(eval $(call gb_UnpackedTarball_add_patches,jfreereport_flute,\
	jfreereport/patches/common_build.patch \
))

# vim: set noet sw=4 ts=4:
