# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,fonts_noto,font_noto))

$(eval $(call gb_ExternalPackage_add_unpacked_files,fonts_noto,$(LIBO_SHARE_FOLDER)/fonts/truetype,\
	NotoKufiArabic-Bold.ttf \
	NotoKufiArabic-Regular.ttf \
	NotoMono-Regular.ttf \
	NotoNaskhArabic-Bold.ttf \
	NotoNaskhArabic-Regular.ttf \
	NotoNaskhArabicUI-Bold.ttf \
	NotoNaskhArabicUI-Regular.ttf \
	NotoSansArabic-Bold.ttf \
	NotoSansArabic-Regular.ttf \
	NotoSansArabicUI-Bold.ttf \
	NotoSansArabicUI-Regular.ttf \
	NotoSansArmenian-Bold.ttf \
	NotoSansArmenian-Regular.ttf \
	NotoSans-Bold.ttf \
	NotoSans-BoldItalic.ttf \
	NotoSans-Condensed.ttf \
	NotoSans-CondensedBold.ttf \
	NotoSans-CondensedBoldItalic.ttf \
	NotoSans-CondensedItalic.ttf \
	NotoSansGeorgian-Bold.ttf \
	NotoSansGeorgian-Regular.ttf \
	NotoSansHebrew-Bold.ttf \
	NotoSansHebrew-Regular.ttf \
	NotoSans-Italic.ttf \
	NotoSansLao-Bold.ttf \
	NotoSansLao-Regular.ttf \
	NotoSans-Light.ttf \
	NotoSans-LightItalic.ttf \
	NotoSansLisu-Regular.ttf \
	NotoSans-Regular.ttf \
	NotoSerifArmenian-Bold.ttf \
	NotoSerifArmenian-Regular.ttf \
	NotoSerif-Bold.ttf \
	NotoSerif-BoldItalic.ttf \
	NotoSerif-Condensed.ttf \
	NotoSerif-CondensedBold.ttf \
	NotoSerif-CondensedBoldItalic.ttf \
	NotoSerif-CondensedItalic.ttf \
	NotoSerifGeorgian-Bold.ttf \
	NotoSerifGeorgian-Regular.ttf \
	NotoSerifHebrew-Bold.ttf \
	NotoSerifHebrew-Regular.ttf \
	NotoSerif-Italic.ttf \
	NotoSerifLao-Bold.ttf \
	NotoSerifLao-Regular.ttf \
	NotoSerif-Light.ttf \
	NotoSerif-LightItalic.ttf \
	NotoSerif-Regular.ttf \
))

# vim: set noet sw=4 ts=4:
