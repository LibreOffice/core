# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,breakpad))

$(eval $(call gb_UnpackedTarball_set_patchlevel,breakpad,0))

$(eval $(call gb_UnpackedTarball_set_tarball,breakpad,$(BREAKPAD_TARBALL)))

# external/breakpad/0001-Handle-race-between-ExceptionHandler-SignalHandler-a.patch upstreamed at
#  <https://chromium-review.googlesource.com/c/breakpad/breakpad/+/1176811> "Handle race between
#  ExceptionHandler::SignalHandler and ~ExceptionHandler":
$(eval $(call gb_UnpackedTarball_add_patches,breakpad,\
	external/breakpad/breakpad-use-correct-http-header.patch.1 \
	external/breakpad/breakpad-wshadow.patch.1 \
	external/breakpad/breakpad-wshadow2.patch.1 \
	external/breakpad/breakpad-stackwalk.patch.1 \
	external/breakpad/ucontext.patch \
	external/breakpad/0001-Handle-race-between-ExceptionHandler-SignalHandler-a.patch.1 \
	external/breakpad/0001-Fix-double-declaration-of-tgkill-when-using-Android-.patch.1 \
	external/breakpad/SIGSTKSZ.patch \
))

ifeq ($(COM_IS_CLANG),TRUE)
ifneq ($(filter -fsanitize=%,$(CC)),)
$(eval $(call gb_UnpackedTarball_add_patches,breakpad, \
    external/breakpad/ubsan.patch \
))
endif
endif

# vim: set noet sw=4 ts=4:
