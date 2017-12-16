# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,fonts_alef,font_alef))

$(eval $(call gb_ExternalPackage_add_unpacked_files,fonts_alef,$(LIBO_SHARE_FOLDER)/fonts/truetype,\
	Alef-Bold.ttf \
	Alef-Regular.ttf \
))

# vim: set noet sw=4 ts=4:
