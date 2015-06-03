# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,udunits2))

$(eval $(call gb_UnpackedTarball_set_tarball,udunits2,$(UDUNITS2_TARBALL)))

ifeq ($(OS),MACOSX)
$(eval $(call gb_UnpackedTarball_add_patches,udunits2,\
	external/udunits2/udunits2-macosx.patch.1 \
))
endif

# vim: set noet sw=4 ts=4:
