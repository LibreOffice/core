# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,rasqal))

$(eval $(call gb_UnpackedTarball_set_tarball,rasqal,$(RASQAL_TARBALL)))

$(eval $(call gb_UnpackedTarball_add_patches,rasqal,\
	redland/rasqal/rasqal-0.9.16.legal.patch \
	redland/rasqal/rasqal-0.9.16.autotools.patch \
	redland/rasqal/rasqal-0.9.16.ooo_build.patch \
	redland/rasqal/rasqal-0.9.16.win32.patch \
	redland/rasqal/rasqal-0.9.16.aix.patch \
	$(if $(filter-out WNT,$(OS)),redland/rasqal/rasqal-0.9.16.bundled-soname.patch) \
	$(if $(filter ANDROID,$(OS)),redland/rasqal/rasqal-0.9.16.no-soname.patch) \
	$(if $(filter WNTGCC,$(OS)$(COM)),redland/rasqal/rasqal-0.9.16.mingw.patch) \
))

# vim: set noet sw=4 ts=4:
