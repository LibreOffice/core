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
$(eval $(call gb_UnpackedTarball_add_file,raptor,src/raptor2.h,redland/raptor/raptor2.h))
$(eval $(call gb_UnpackedTarball_add_file,raptor,src/raptor_config.h,redland/raptor/raptor_config.h))

$(eval $(call gb_UnpackedTarball_add_patches,raptor,\
	redland/raptor/raptor-freebsd.patch.1 \
	$(if $(filter WNTGCC,$(OS)$(COM)),redland/raptor/raptor-mingw.patch.1) \
	$(if $(filter-out WNT,$(OS)),redland/raptor/raptor-bundled-soname.patch.1) \
	$(if $(filter ANDROID,$(OS)),redland/raptor/raptor-android.patch.1) \
))

# vim: set noet sw=4 ts=4:
