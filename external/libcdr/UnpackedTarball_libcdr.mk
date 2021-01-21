# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,libcdr))

$(eval $(call gb_UnpackedTarball_set_tarball,libcdr,$(CDR_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,libcdr,0))

$(eval $(call gb_UnpackedTarball_update_autoconf_configs,libcdr))

$(eval $(call gb_UnpackedTarball_add_patches,libcdr, \
    external/libcdr/libcdr-visibility-win.patch \
    external/libcdr/libcdr-no-icu-boolean.patch.1 \
    external/libcdr/0001-ax_gcc_func_attribute-Revise-the-detection-of-unknow.patch.1 \
))

# vim: set noet sw=4 ts=4:
