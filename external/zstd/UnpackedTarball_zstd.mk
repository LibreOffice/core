# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,zstd))

$(eval $(call gb_UnpackedTarball_set_tarball,zstd,$(ZSTD_TARBALL)))

ifneq ($(MSYSTEM),)
# the tarball contains effectively dangling symlinks (links to files extracted later)
# that breaks in git-bash/msys since MSYS defaults to not create links
ifeq ($(filter winsymlinks%,$(MSYS)),)
$(call gb_UnpackedTarball_get_target,zstd): export MSYS:=$(MSYS) winsymlinks
endif
endif

# vim: set noet sw=4 ts=4:
