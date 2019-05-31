# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,libqrgen))

$(eval $(call gb_UnpackedTarball_set_tarball,libqrgen,$(QRGEN_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,libqrgen,0))

$(eval $(call gb_UnpackedTarball_update_autoconf_configs,libqrgen))