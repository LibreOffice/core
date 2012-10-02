# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,visio))

$(eval $(call gb_UnpackedTarball_set_tarball,visio,$(VISIO_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,visio,0))

visio_patches :=
# Somehow draw is very picky about what is a closed path
visio_patches += libvisio-0.0.19.patch

$(eval $(call gb_UnpackedTarball_add_patches,visio,\
	$(foreach patch,$(visio_patches),libvisio/$(patch)) \
))
# vim: set noet sw=4 ts=4:
