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
	icu/icu4c.9948.mlym-crash.patch \
	icu/icu4c-bsd.patch \
	icu/icu4c-build.patch \
	icu/icu4c.8198.revert.icu5431.patch \
	icu/icu4c.7601.Indic-ccmp.patch \
	icu/icu4c-aix.patch \
	icu/icu4c-wchar_t.patch \
	icu/icu4c-warnings.patch \
	icu/icu4c.9313.cygwin.patch \
	icu/icu4c-macosx.patch \
	icu/icu4c-solarisgcc.patch \
	icu/icu4c-mkdir.patch \
	icu/icu4c-buffer-overflow.patch \
	icu/icu4c-$(if $(filter ANDROID,$(OS)),android,rpath).patch \
))

# vim: set noet sw=4 ts=4:
