# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,font_gentium_inc,font_gentium))

$(eval $(call gb_ExternalPackage_add_unpacked_files,font_gentium_inc,pck,\
	GenBasB.ttf \
	GenBasBI.ttf \
	GenBasI.ttf \
	GenBasR.ttf \
	GenBkBasB.ttf \
	GenBkBasBI.ttf \
	GenBkBasI.ttf \
	GenBkBasR.ttf \
))

$(eval $(call gb_ExternalPackage_add_unpacked_file,font_gentium_inc,pck/License_gentium.txt,OFL.txt))

# vim: set noet sw=4 ts=4:
