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
	icu/icu5c.9948.mlym-crash.patch \
	icu/icu5c-build.patch \
	icu/icu5c.8198.revert.icu5431.patch \
	icu/icu5c.7601.Indic-ccmp.patch \
	icu/icu5c-aix.patch \
	icu/icu5c-wchar_t.patch \
	icu/icu5c-warnings.patch \
	icu/icu5c-macosx.patch \
	icu/icu5c-solarisgcc.patch \
	icu/icu5c-mkdir.patch \
	icu/icu5c-buffer-overflow.patch \
	icu/icu5c-$(if $(filter ANDROID,$(OS)),android,rpath).patch \
))

# vim: set noet sw=4 ts=4:
