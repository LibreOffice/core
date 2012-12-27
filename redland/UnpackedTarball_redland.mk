# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,redland))

$(eval $(call gb_UnpackedTarball_set_tarball,redland,$(REDLAND_TARBALL)))

$(eval $(call gb_UnpackedTarball_add_patches,redland,\
	redland/redland/redland-1.0.8.legal.patch \
	redland/redland/redland-1.0.8.autotools.patch \
	redland/redland/redland-1.0.8.ooo_build.patch \
	redland/redland/redland-1.0.8.win32.patch \
	redland/redland/redland-1.0.8.aix.patch \
	$(if $(filter-out WNT,$(OS)),redland/redland/redland-1.0.8.bundled-soname.patch) \
	$(if $(filter ANDROID,$(OS)),redland/redland/redland-1.0.8.no-soname.patch) \
	$(if $(filter WNTGCC,$(OS)$(COM)),redland/redland/redland-1.0.8.mingw.patch) \
))

# vim: set noet sw=4 ts=4:
