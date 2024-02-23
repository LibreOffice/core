# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,fonts_noto_sans,font_noto_sans))

$(eval $(call gb_ExternalPackage_add_unpacked_files,fonts_noto_sans,$(LIBO_SHARE_FOLDER)/fonts/truetype,\
	NotoSans/full/ttf/NotoSans-Bold.ttf \
	NotoSans/full/ttf/NotoSans-BoldItalic.ttf \
	NotoSans/full/ttf/NotoSans-Italic.ttf \
	NotoSans/full/ttf/NotoSans-Regular.ttf \
	NotoSans/full/ttf/NotoSans-Condensed.ttf \
	NotoSans/full/ttf/NotoSans-CondensedBold.ttf \
	NotoSans/full/ttf/NotoSans-CondensedBoldItalic.ttf \
	NotoSans/full/ttf/NotoSans-CondensedItalic.ttf \
	NotoSans/full/ttf/NotoSans-Light.ttf \
	NotoSans/full/ttf/NotoSans-LightItalic.ttf \
))

# vim: set noet sw=4 ts=4:
