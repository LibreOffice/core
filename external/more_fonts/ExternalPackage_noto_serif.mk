# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,fonts_noto_serif,font_noto_serif))

$(eval $(call gb_ExternalPackage_add_unpacked_files,fonts_noto_serif,$(LIBO_SHARE_FOLDER)/fonts/truetype,\
	NotoSerif/hinted/ttf/NotoSerif-Bold.ttf \
	NotoSerif/hinted/ttf/NotoSerif-BoldItalic.ttf \
	NotoSerif/hinted/ttf/NotoSerif-Italic.ttf \
	NotoSerif/hinted/ttf/NotoSerif-Regular.ttf \
	NotoSerif/hinted/ttf/NotoSerif-Condensed.ttf \
	NotoSerif/hinted/ttf/NotoSerif-CondensedBold.ttf \
	NotoSerif/hinted/ttf/NotoSerif-CondensedBoldItalic.ttf \
	NotoSerif/hinted/ttf/NotoSerif-CondensedItalic.ttf \
	NotoSerif/hinted/ttf/NotoSerif-Light.ttf \
	NotoSerif/hinted/ttf/NotoSerif-LightItalic.ttf \
))

# vim: set noet sw=4 ts=4:
