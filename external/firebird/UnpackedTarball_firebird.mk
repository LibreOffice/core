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
		external/firebird/Provide-sized-global-delete-operators-when-compiled-.patch \
		external/firebird/Make-the-generated-code-compatible-with-gcc-6-in-C-1.patch \
        external/firebird/firebird.disable-ib-util-not-found.patch.1 \
		external/firebird/firebird-Engine12.patch \
		external/firebird/firebird-rpath.patch.0 \
		external/firebird/firebird-cloop-compiler.patch.1 \
		external/firebird/wnt-dbgutil.patch \
		external/firebird/debug-gfix-failure.patch \
		external/firebird/libc++.patch \
		external/firebird/os_utils-O_CLOEXEC.patch \
		external/firebird/remote-inet-SOL_TCP.patch \
		external/firebird/my-own-version-of-strnlen.patch \
))

ifeq ($(OS)-$(COM),WNT-MSC)
$(eval $(call gb_UnpackedTarball_add_patches,firebird,\
	external/firebird/firebird-cygwin-msvc.patch \
))
endif

ifeq ($(OS),MACOSX)
$(eval $(call gb_UnpackedTarball_add_patches,firebird,\
	external/firebird/firebird-macosx.patch.1 \
	external/firebird/firebird-macosx-print-dyldlibpath.patch \
	external/firebird/InputDevices-darwin.patch \
))
ifeq ($(CPUNAME),POWERPC)
$(eval $(call gb_UnpackedTarball_add_patches,firebird,\
	external/firebird/lSystemStubs-darwin.patch \
	external/firebird/prefix.darwin_powerpc.patch \
	external/firebird/semaphore.h-darwin.patch \
))
else
$(eval $(call gb_UnpackedTarball_add_patches,firebird,\
	external/firebird/firebird-configure-x86-64-macosx.patch.1 \
))
endif
endif
# vim: set noet sw=4 ts=4:
