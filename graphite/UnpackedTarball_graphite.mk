# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,graphite))

$(eval $(call gb_UnpackedTarball_set_tarball,graphite,$(GRAPHITE_TARBALL)))

# http://projects.palaso.org/issues/1030
# http://projects.palaso.org/issues/1115
$(eval $(call gb_UnpackedTarball_add_patches,graphite,\
	graphite/graphite2.patch \
	graphite/graphite2.issue1115.patch.1 \
))

# vim: set noet sw=4 ts=4:
