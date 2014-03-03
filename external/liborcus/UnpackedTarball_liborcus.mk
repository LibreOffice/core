# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,liborcus))

$(eval $(call gb_UnpackedTarball_set_tarball,liborcus,$(ORCUS_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,liborcus,0))

liborcus_patches :=

# make config.sub recognize arm-linux-androideabi
# liborcus_patches += liborcus_0.1.0-configure.patch

# don't use dllimport
liborcus_patches += liborcus_0.1.0-dllimport.patch

ifneq ($(HAVE_GCC_VISIBILITY_FEATURE),TRUE)
liborcus_patches += visibility.patch
endif

# <https://gitorious.org/orcus/orcus/merge_requests/2#
# f60d6eecee72349993a392a9a63ddf3383d3b8c8-
# f60d6eecee72349993a392a9a63ddf3383d3b8c8@2>:

$(eval $(call gb_UnpackedTarball_add_patches,liborcus,\
	$(foreach patch,$(liborcus_patches),external/liborcus/$(patch)) \
))

# vim: set noet sw=4 ts=4:
