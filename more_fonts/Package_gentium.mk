# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,font_gentium_inc,$(call gb_UnpackedTarball_get_dir,font_gentium)))

$(eval $(call gb_Package_use_unpacked,font_gentium_inc,font_gentium))

$(eval $(call gb_Package_add_files,font_gentium_inc,pck,\
	GenBasB.ttf \
	GenBasBI.ttf \
	GenBasI.ttf \
	GenBasR.ttf \
	GenBkBasB.ttf \
	GenBkBasBI.ttf \
	GenBkBasI.ttf \
	GenBkBasR.ttf \
))

$(eval $(call gb_Package_add_file,font_gentium_inc,pck/License_gentium.txt,OFL.txt))
