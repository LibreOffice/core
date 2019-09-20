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

# * external/libcdr/0001-Add-missing-include.patch.1 is from upstream master (see content for
#   details);
# * external/libcdr/ubsan.patch is upstream at <https://gerrit.libreoffice.org/#/c/73182/> "Avoid UB
#   converting from double to int via unsigned":
$(eval $(call gb_UnpackedTarball_add_patches,libcdr, \
    external/libcdr/libcdr-visibility-win.patch \
    external/libcdr/0001-Add-missing-include.patch.1 \
    external/libcdr/ubsan.patch \
))

# vim: set noet sw=4 ts=4:
