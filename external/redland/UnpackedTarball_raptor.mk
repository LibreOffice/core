# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,raptor))

$(eval $(call gb_UnpackedTarball_set_tarball,raptor,$(RAPTOR_TARBALL),,redland))

# configure generated files for MSVC
$(eval $(call gb_UnpackedTarball_add_file,raptor,src/raptor2.h,external/redland/raptor/raptor2.h))
$(eval $(call gb_UnpackedTarball_add_file,raptor,src/raptor_config.h,external/redland/raptor/raptor_config.h))

$(eval $(call gb_UnpackedTarball_set_patchlevel,raptor,0))

$(eval $(call gb_UnpackedTarball_add_patches,raptor,\
	external/redland/raptor/raptor-freebsd.patch.1 \
	external/redland/raptor/raptor-msvc.patch.1 \
	$(if $(filter-out WNT,$(OS)),external/redland/raptor/raptor-bundled-soname.patch.1) \
	$(if $(filter ANDROID,$(OS)),external/redland/raptor/raptor-android.patch.1) \
	external/redland/raptor/ubsan.patch \
	$(if $(SYSTEM_LIBXML),,external/redland/raptor/rpath.patch) \
	external/redland/raptor/xml2-config.patch \
))

# vim: set noet sw=4 ts=4:
