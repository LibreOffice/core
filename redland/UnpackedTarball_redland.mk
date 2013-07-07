# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,redland))

$(eval $(call gb_UnpackedTarball_set_tarball,redland,$(REDLAND_TARBALL),,redland))

# configure generated files for MSVC
$(eval $(call gb_UnpackedTarball_add_file,redland,src/librdf.h,redland/redland/librdf.h))

$(eval $(call gb_UnpackedTarball_add_patches,redland,\
	redland/redland/redland-query-rasqal-avoid-assert.patch.1 \
	redland/redland/redland-pkgconfig.patch.1 \
	redland/redland/redland-freebsd.patch.1 \
	redland/redland/redland-msvc.patch.1 \
	$(if $(filter-out WNT,$(OS)),redland/redland/redland-bundled-soname.patch.1) \
	$(if $(filter ANDROID,$(OS)),redland/redland/redland-android.patch.1) \
	$(if $(filter WNTGCC,$(OS)$(COM)),redland/redland/redland-mingw.patch.1) \
	$(if $(filter YES,$(CROSS_COMPILING)),redland/redland/redland-xcompile.patch.1) \
))

# vim: set noet sw=4 ts=4:
