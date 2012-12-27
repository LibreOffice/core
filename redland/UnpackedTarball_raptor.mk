# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,raptor))

$(eval $(call gb_UnpackedTarball_set_tarball,raptor,$(RAPTOR_TARBALL)))

$(eval $(call gb_UnpackedTarball_add_patches,raptor,\
	redland/raptor/raptor-1.4.18.legal.patch \
	redland/raptor/raptor-1.4.18.autotools.patch \
	redland/raptor/raptor-1.4.18.ooo_build.patch \
	redland/raptor/raptor-1.4.18.win32.patch \
	redland/raptor/raptor-1.4.18.fixes.patch \
	redland/raptor/raptor-1.4.18.rindex.patch \
	redland/raptor/raptor-1.4.18.aix.patch \
	redland/raptor/raptor-1.4.18.entities.patch \
	$(if $(filter-out WNT,$(OS)),redland/raptor/raptor-1.4.18.bundled-soname.patch) \
	$(if $(filter YES,$(CROSS_COMPILING)),redland/raptor/raptor-1.4.18.cross.patch) \
	$(if $(filter YES,$(SYSTEM_LIBXML)),redland/raptor/raptor-1.4.18.libxml.patch) \
	$(if $(filter YES,$(SYSTEM_LIBXSLT)),redland/raptor/raptor-1.4.18.libxslt.patch) \
	$(if $(filter ANDROID,$(OS)),redland/raptor/raptor-1.4.18.no-soname.patch) \
	$(if $(filter WNTGCC,$(OS)$(COM)),redland/raptor/raptor-1.4.18.mingw.patch) \
))

# vim: set noet sw=4 ts=4:
