# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,icu))

$(eval $(call gb_UnpackedTarball_set_tarball,icu,$(ICU_TARBALL)))

$(eval $(call gb_UnpackedTarball_add_patches,icu,\
	external/icu/icu4c-build.patch \
	external/icu/icu4c-aix.patch \
	external/icu/icu4c-wchar_t.patch \
	external/icu/icu4c-warnings.patch \
	external/icu/icu4c-macosx.patch \
	external/icu/icu4c-solarisgcc.patch \
	external/icu/icu4c-mkdir.patch \
	external/icu/icu4c-buffer-overflow.patch \
	external/icu/icu4c-$(if $(filter ANDROID,$(OS)),android,rpath).patch \
	external/icu/icu4c-ubsan.patch.0 \
))

# vim: set noet sw=4 ts=4:
