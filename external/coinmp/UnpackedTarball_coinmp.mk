# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,coinmp))

$(eval $(call gb_UnpackedTarball_set_tarball,coinmp,$(COINMP_TARBALL)))

$(eval $(call gb_UnpackedTarball_fix_end_of_line,coinmp,\
	CoinMP/MSVisualStudio/v9/CoinMP.sln \
))

$(eval $(call gb_UnpackedTarball_set_patchlevel,coinmp,0))

$(eval $(call gb_UnpackedTarball_update_autoconf_configs,coinmp))
$(eval $(call gb_UnpackedTarball_update_autoconf_configs,coinmp,\
	BuildTools \
	Cbc \
	Cgl \
	Clp \
	CoinMP \
	CoinUtils \
	Data/Sample \
	Osi \
))

# * external/coinmp/Wnon-c-typedef-for-linkage.patch upstream at
#   <https://list.coin-or.org/pipermail/coin-discuss/2020-February/003972.html> "[Coin-discuss]
#   Small patch to fix Clang -Wnon-c-typedef-for-linkage in Clp":
$(eval $(call gb_UnpackedTarball_add_patches,coinmp,\
	external/coinmp/osi_cuts_iterator.patch.0 \
	external/coinmp/no-binaries.patch.1 \
	external/coinmp/werror-format-security.patch.0 \
	external/coinmp/werror-undef.patch.0 \
	external/coinmp/coinmp-msvc-disable-sse2.patch.1 \
	$(if $(filter MSC,$(COM)),external/coinmp/windows.build.patch.1) \
	external/coinmp/werror-format-pedantic.patch.0 \
	external/coinmp/ubsan.patch.0 \
	external/coinmp/rpath.patch \
	external/coinmp/libtool.patch \
	external/coinmp/Wnon-c-typedef-for-linkage.patch \
	external/coinmp/register.patch \
	external/coinmp/configure-exit.patch \
))

# vim: set noet sw=4 ts=4:
