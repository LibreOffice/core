# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,libgpg-error))

$(eval $(call gb_UnpackedTarball_set_tarball,libgpg-error,$(LIBGPGERROR_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,libgpg-error,0))

$(eval $(call gb_UnpackedTarball_add_patches,libgpg-error, \
	$(if $(filter MSC,$(COM)),external/libgpg-error/w32-build-fixes.patch) \
	$(if $(filter MSC,$(COM)),external/libgpg-error/w32-build-fixes-2.patch.1) \
	$(if $(filter MSC,$(COM)),external/libgpg-error/w32-build-fixes-3.patch.1) \
	$(if $(filter MSC,$(COM)),external/libgpg-error/w32-disable-dllinit.patch.1) \
	external/libgpg-error/w32-build-fixes-4.patch \
	$(if $(filter MSC,$(COM)),external/libgpg-error/w32-build-fixes-5.patch) \
	$(if $(filter LINUX,$(OS)),external/libgpg-error/libgpgerror-bundled-soname.patch.1) \
	external/libgpg-error/clang-cl.patch \
))

# vim: set noet sw=4 ts=4:
