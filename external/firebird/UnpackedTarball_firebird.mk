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

$(eval $(call gb_UnpackedTarball_add_patches,firebird,\
        external/firebird/firebird.disable-ib-util-not-found.patch.1 \
		external/firebird/firebird-Engine12.patch \
		external/firebird/firebird-rpath.patch.0 \
		external/firebird/firebird-cloop-compiler.patch.1 \
		external/firebird/firebird-gcc6.patch.1 \
		external/firebird/wnt-dbgutil.patch \
		external/firebird/libc++.patch \
		external/firebird/0001-Avoid-hangup-in-SS-when-error-happens-at-system-atta.patch.1 \
		external/firebird/0002-Backported-fix-for-CORE-5452-Segfault-when-engine-s-.patch.1 \
))

ifeq ($(OS),WNT)
$(eval $(call gb_UnpackedTarball_add_patches,firebird,\
	external/firebird/firebird-cygwin-msvc.patch \
))
endif

ifeq ($(OS),MACOSX)
$(eval $(call gb_UnpackedTarball_add_patches,firebird,\
	external/firebird/firebird-configure-x86-64-macosx.patch.1 \
	external/firebird/firebird-macosx.patch.1 \
	external/firebird/macosx-elcapitan-dyld.patch \
))
endif

# vim: set noet sw=4 ts=4:
