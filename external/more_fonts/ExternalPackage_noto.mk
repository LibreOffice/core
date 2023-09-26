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
	NotoSansGeorgian-Bold.ttf \
	NotoSansGeorgian-Regular.ttf \
	NotoSansLao-Bold.ttf \
	NotoSansLao-Regular.ttf \
	NotoSansLisu-Regular.ttf \
	NotoSerifArmenian-Bold.ttf \
	NotoSerifArmenian-Regular.ttf \
	NotoSerifGeorgian-Bold.ttf \
	NotoSerifGeorgian-Regular.ttf \
	NotoSerifLao-Bold.ttf \
	NotoSerifLao-Regular.ttf \
))

# vim: set noet sw=4 ts=4:
