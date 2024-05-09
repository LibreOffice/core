# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,librevenge))

$(eval $(call gb_UnpackedTarball_set_tarball,librevenge,$(REVENGE_TARBALL)))

$(eval $(call gb_UnpackedTarball_update_autoconf_configs,librevenge))

$(eval $(call gb_UnpackedTarball_set_patchlevel,librevenge,0))

$(eval $(call gb_UnpackedTarball_add_patches,librevenge, \
    external/librevenge/rpath.patch \
    external/librevenge/tdf63130-improve-perf.patch.0 \
    external/librevenge/C2039-unique-shared_ptr.patch.0 \
))

ifneq ($(OS),MACOSX)
ifneq ($(OS),WNT)
$(eval $(call gb_UnpackedTarball_add_patches,librevenge, \
    external/librevenge/librevenge-bundled-soname.patch.0 \
))
endif
endif

# vim: set noet sw=4 ts=4:
