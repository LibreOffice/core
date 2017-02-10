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
$(eval $(call gb_UnpackedTarball_add_file,redland,src/librdf.h,external/redland/redland/librdf.h))

$(eval $(call gb_UnpackedTarball_set_patchlevel,redland,0))

# redland-format.patch.0 sent upstream as
#  <https://github.com/dajobe/librdf/pull/6>
$(eval $(call gb_UnpackedTarball_add_patches,redland,\
	external/redland/redland/redland-freebsd.patch.1 \
	external/redland/redland/redland-msvc.patch.1 \
	$(if $(filter-out WNT,$(OS)),external/redland/redland/redland-bundled-soname.patch.1) \
	$(if $(filter ANDROID,$(OS)),external/redland/redland/redland-android.patch.1) \
	$(if $(CROSS_COMPILING),external/redland/redland/redland-xcompile.patch.1) \
	external/redland/redland/redland-format.patch.0 \
	external/redland/redland/rpath.patch \
	external/redland/redland/clang-cl.patch \
))

# vim: set noet sw=4 ts=4:
