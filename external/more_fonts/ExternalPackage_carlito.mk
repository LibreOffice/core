# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,fonts_carlito,font_carlito))

$(eval $(call gb_ExternalPackage_add_unpacked_files,fonts_carlito,$(LIBO_SHARE_FOLDER)/fonts/truetype,\
	Carlito-BoldItalic.ttf \
	Carlito-Regular.ttf \
	Carlito-Italic.ttf \
	Carlito-Bold.ttf \
))

# vim: set noet sw=4 ts=4:
