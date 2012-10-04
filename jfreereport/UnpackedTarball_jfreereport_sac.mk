# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,jfreereport_sac))

$(eval $(call gb_UnpackedTarball_set_tarball,jfreereport_sac,$(JFREEREPORT_SAC_TARBALL)))

$(eval $(call gb_UnpackedTarball_add_file,jfreereport_sac,build.xml,jfreereport/java/sac/build.xml))

# vim: set noet sw=4 ts=4:
