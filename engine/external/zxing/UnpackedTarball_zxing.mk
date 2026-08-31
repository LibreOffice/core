#-*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,zxing))

$(eval $(call gb_UnpackedTarball_set_tarball,zxing,$(ZXING_TARBALL)))

# external/zxing/0001-const-up-some-symbols.patch
# upstream effort at: https://github.com/zxing-cpp/zxing-cpp/pull/979

ifneq ($(MSYSTEM),)
# the 2.3.0 tarball contains dangling symlinks (to a submodule component/experimental backend)
# git-bash/msys tar fails when extracting since MSYS defaults to not create those; the
# nativestrict flavor cannot create them either (a native symlink to a missing target is
# refused, at least by some msys runtime versions). Force the shortcut-style mode for this
# one unpack: it never needs the target, and nothing in the build follows these links.
$(call gb_UnpackedTarball_get_target,zxing): export MSYS:=$(filter-out winsymlinks%,$(MSYS)) winsymlinks
endif

$(eval $(call gb_UnpackedTarball_add_patches,zxing, \
	external/zxing/0001-add-ZXVersion-h.patch \
	external/zxing/0001-const-up-some-symbols.patch \
))

# vim: set noet sw=4 ts=4:
