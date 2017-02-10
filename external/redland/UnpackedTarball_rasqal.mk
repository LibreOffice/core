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
$(eval $(call gb_UnpackedTarball_add_file,rasqal,src/rasqal.h,external/redland/rasqal/rasqal.h))

$(eval $(call gb_UnpackedTarball_set_patchlevel,rasqal,0))

$(eval $(call gb_UnpackedTarball_add_patches,rasqal,\
	external/redland/rasqal/rasqal-pkgconfig.patch.1 \
	external/redland/rasqal/rasqal-freebsd.patch.1 \
	external/redland/rasqal/rasqal-msvc.patch.1 \
	external/redland/rasqal/rasqal-aix.patch.1 \
	$(if $(filter-out WNT,$(OS)),external/redland/rasqal/rasqal-bundled-soname.patch.1) \
	$(if $(filter ANDROID,$(OS)),external/redland/rasqal/rasqal-android.patch.1) \
	$(if $(CROSS_COMPILING),external/redland/rasqal/rasqal-xcompile.patch.1) \
	external/redland/rasqal/rpath.patch \
	external/redland/rasqal/clang-cl.patch \
))

# vim: set noet sw=4 ts=4:
