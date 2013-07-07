# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,rhino))

$(eval $(call gb_UnpackedTarball_set_tarball,rhino,$(RHINO_TARBALL),,rhino))

$(eval $(call gb_UnpackedTarball_set_patchlevel,rhino,2))

$(eval $(call gb_UnpackedTarball_add_patches,rhino,\
	rhino/rhino1_5R5.patch \
	rhino/rhino1_5R5-find_swing.patch \
	rhino/rhino1_5R5-updateToolTip.patch \
))

$(eval $(call gb_UnpackedTarball_add_file,rhino,toolsrc/org/mozilla/javascript/tools/debugger/OfficeScriptInfo.java,rhino/OfficeScriptInfo.java))

# vim: set noet sw=4 ts=4:
