# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,soffice_bin))

$(eval $(call gb_Executable_set_include,soffice_bin,\
    $$(INCLUDE) \
    -I$(SRCDIR)/desktop/source/inc \
))

$(eval $(call gb_Executable_use_libraries,soffice_bin,\
    sal \
    sofficeapp \
))

$(eval $(call gb_Executable_add_cobjects,soffice_bin,\
    desktop/source/app/main \
))

ifeq ($(OS),MACOSX)

$(eval $(call gb_Executable_set_ldflags,\
    $(filter-out -bind_at_load,$$(LDFLAGS)) \
))

endif

ifeq ($(OS)-$(HAVE_MACOS_LD_PLATFORMVERSION),MACOSX-TRUE)
# At least when building against SDK 10.15, changing the LC_VERSION_MIN_MACOSX load command's sdk
# value from 10.15 to "n/a" (i.e., 0.0.0) is necessary to avoid blurry text in the LO UI (see
# <https://github.com/llvm/llvm-project/commit/25ce33a6e4f3b13732c0f851e68390dc2acb9123>
# "[driver][darwin] Pass -platform_version flag to the linker instead of the
# -<platform>_version_min flag", clang/test/Driver/darwin-ld-platform-version-macos.c in particular,
# for the -platform_version that Clang passes by default to new-enough ld):
$(eval $(call gb_Executable_add_ldflags,soffice_bin, \
    -Xlinker -platform_version -Xlinker macos -Xlinker $(MAC_OS_X_VERSION_MIN_REQUIRED_DOTS) \
        -Xlinker 0.0.0 \
))
endif

ifeq ($(OS),WNT)

$(eval $(call gb_Executable_set_targettype_gui,soffice_bin,NO))

$(eval $(call gb_Executable_add_nativeres,soffice_bin,sofficebin/officeloader))

ifeq ($(COM),MSC)

$(eval $(call gb_Executable_add_ldflags,soffice_bin,\
    /STACK:10000000 \
))

endif

endif

# vim: set ts=4 sw=4 et:
