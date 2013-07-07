# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,rasqal))

$(eval $(call gb_UnpackedTarball_set_tarball,rasqal,$(RASQAL_TARBALL),,redland))

# configure generated files for MSVC
$(eval $(call gb_UnpackedTarball_add_file,rasqal,src/rasqal.h,redland/rasqal/rasqal.h))

$(eval $(call gb_UnpackedTarball_add_patches,rasqal,\
	redland/rasqal/rasqal-pkgconfig.patch.1 \
	redland/rasqal/rasqal-freebsd.patch.1 \
	redland/rasqal/rasqal-msvc.patch.1 \
	redland/rasqal/rasqal-aix.patch.1 \
	$(if $(filter-out WNT,$(OS)),redland/rasqal/rasqal-bundled-soname.patch.1) \
	$(if $(filter ANDROID,$(OS)),redland/rasqal/rasqal-android.patch.1) \
	$(if $(filter WNTGCC,$(OS)$(COM)),redland/rasqal/rasqal-mingw.patch.1) \
	$(if $(filter YES,$(CROSS_COMPILING)),redland/rasqal/rasqal-xcompile.patch.1) \
))

# vim: set noet sw=4 ts=4:
