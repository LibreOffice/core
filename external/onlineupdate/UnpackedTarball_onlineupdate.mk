# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,onlineupdate))

$(eval $(call gb_UnpackedTarball_set_tarball,onlineupdate,$(ONLINEUPDATE_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,onlineupdate,0))

$(eval $(call gb_UnpackedTarball_add_patches,onlineupdate, \
    external/onlineupdate/gtk3deprecated.patch \
    external/onlineupdate/lo.patch \
))

ifeq ($(OS),WNT)
$(eval $(call gb_UnpackedTarball_add_patches,onlineupdate, \
    external/onlineupdate/cygpath.patch \
))
endif

# The update maintenance service that is used on Windows has a couple of checks that files in the
# to-be-updated installation set are signed, which would fail for --disable-windows-build-signing;
# so, as a HACK for debugging purposes, silence those problematic checks for --enable-dbgutil:
ifeq ($(OS)-$(WINDOWS_BUILD_SIGNING)-$(ENABLE_DBGUTIL),WNT-FALSE-TRUE)
$(eval $(call gb_UnpackedTarball_add_patches,onlineupdate, \
    external/onlineupdate/unsigned-hack.patch \
))
endif

# vim: set noet sw=4 ts=4:
