# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

epubgen_patches :=
# Backport of <https://sourceforge.net/p/libepubgen/code/ci/006848cb62225647c418d5143d4e88a9d73829da/>.
epubgen_patches += libepubgen-epub3.patch.1
# Backport of <https://sourceforge.net/p/libepubgen/code/ci/9a284081eea4a95235a6d6a6a50cbe3f7ad323ba/>.
epubgen_patches += 0001-Support-writing-mode-for-reflowable-layout-method.patch.1
# Backport of <https://sourceforge.net/p/libepubgen/code/ci/0318031b9094b9180d1d391d0ca31a782b016e99/>.
epubgen_patches += 0002-Always-keep-page-properties-when-splitting-the-HTML-.patch.1
# Backport of <https://sourceforge.net/p/libepubgen/code/ci/1f602fcaa74fc9dbc6457019d11c602ff4040a4e/>.
epubgen_patches += 0003-Ensure-page-properties-in-the-page-span-works.patch.1

ifeq ($(COM_IS_CLANG),TRUE)
ifneq ($(filter -fsanitize=%,$(CC)),)
epubgen_patches += ubsan-visibility.patch
endif
endif

$(eval $(call gb_UnpackedTarball_UnpackedTarball,libepubgen))

$(eval $(call gb_UnpackedTarball_set_tarball,libepubgen,$(EPUBGEN_TARBALL)))

$(eval $(call gb_UnpackedTarball_update_autoconf_configs,libepubgen))

$(eval $(call gb_UnpackedTarball_set_patchlevel,libepubgen,0))

$(eval $(call gb_UnpackedTarball_add_patches,libepubgen,\
	$(foreach patch,$(epubgen_patches),external/libepubgen/$(patch)) \
))

# vim: set noet sw=4 ts=4:
