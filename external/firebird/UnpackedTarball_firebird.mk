# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,firebird))

$(eval $(call gb_UnpackedTarball_set_tarball,firebird,$(FIREBIRD_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,firebird,0))

$(eval $(call gb_UnpackedTarball_update_autoconf_configs,firebird,\
	builds/make.new/config \
	extern/editline \
))

# * external/firebird/0001-Make-comparison-operator-member-functions-const.patch.1 is upstream at
#   <https://github.com/FirebirdSQL/firebird/pull/227> "Make comparison operator member functions
#   const";
# * external/firebird/0001-Fix-checks-for-null-HANDLE-in-Windows-only-code.patch.1 is upstream at
#   <https://github.com/FirebirdSQL/firebird/pull/301> "Fix checks for null HANDLE in Windows-only
#   code",
# * external/firebird/0001-extern-cloop-Missing-dependency-of-BIN_DIR-cloop-on-.patch.1 is upstream
#   at <https://github.com/FirebirdSQL/firebird/pull/302> "extern/cloop: Missing dependency of
#   $(BIN_DIR)/cloop on $(BIN_DIR)":
$(eval $(call gb_UnpackedTarball_add_patches,firebird,\
        external/firebird/firebird.disable-ib-util-not-found.patch.1 \
		external/firebird/firebird-Engine12.patch \
		external/firebird/firebird-rpath.patch.0 \
		external/firebird/wnt-dbgutil.patch \
		external/firebird/c++17.patch \
		external/firebird/ubsan.patch \
		external/firebird/asan.patch \
		external/firebird/firebird-tdf125284.patch.1 \
		external/firebird/0001-Make-comparison-operator-member-functions-const.patch.1 \
    external/firebird/0001-Fix-warning-on-Win64-build-231.patch.1 \
		external/firebird/macos-arm64.patch.0 \
    external/firebird/firebird-btyacc-add-explicit-rule.patch \
    external/firebird/firebird-307.patch.1 \
    external/firebird/0001-Fix-checks-for-null-HANDLE-in-Windows-only-code.patch.1 \
    external/firebird/0001-extern-cloop-Missing-dependency-of-BIN_DIR-cloop-on-.patch.1 \
    external/firebird/msvc.patch \
))

ifeq ($(OS),WNT)
$(eval $(call gb_UnpackedTarball_add_patches,firebird,\
	external/firebird/firebird-cygwin-msvc.patch \
	external/firebird/firebird-cygwin-msvc-warnings.patch \
	external/firebird/firebird-vs2017.patch.1 \
))
endif

ifeq ($(OS),MACOSX)
$(eval $(call gb_UnpackedTarball_add_patches,firebird,\
	external/firebird/firebird-macosx.patch.1 \
	external/firebird/macosx-elcapitan-dyld.patch \
))
endif

ifneq ($(filter -fsanitize=%,$(CC)),)
$(eval $(call gb_UnpackedTarball_add_patches,firebird, \
    external/firebird/sanitizer.patch \
))
endif

# vim: set noet sw=4 ts=4:
