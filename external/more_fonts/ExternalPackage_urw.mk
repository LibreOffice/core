# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,fonts_urw,font_urw))

$(eval $(call gb_ExternalPackage_add_unpacked_files,fonts_urw,$(LIBO_SHARE_FOLDER)/fonts/truetype,\
	fonts/C059-BdIta.otf \
	fonts/C059-Bold.otf \
	fonts/C059-Italic.otf \
	fonts/C059-Roman.otf \
	fonts/D050000L.otf \
	fonts/NimbusMonoPS-BoldItalic.otf \
	fonts/NimbusMonoPS-Bold.otf \
	fonts/NimbusMonoPS-Italic.otf \
	fonts/NimbusMonoPS-Regular.otf \
	fonts/NimbusRoman-BoldItalic.otf \
	fonts/NimbusRoman-Bold.otf \
	fonts/NimbusRoman-Italic.otf \
	fonts/NimbusRoman-Regular.otf \
	fonts/NimbusSans-BoldItalic.otf \
	fonts/NimbusSans-Bold.otf \
	fonts/NimbusSans-Italic.otf \
	fonts/NimbusSansNarrow-BdOblique.otf \
	fonts/NimbusSansNarrow-Bold.otf \
	fonts/NimbusSansNarrow-Oblique.otf \
	fonts/NimbusSansNarrow-Regular.otf \
	fonts/NimbusSans-Regular.otf \
	fonts/P052-BoldItalic.otf \
	fonts/P052-Bold.otf \
	fonts/P052-Italic.otf \
	fonts/P052-Roman.otf \
	fonts/StandardSymbolsPS.otf \
	fonts/URWBookman-DemiItalic.otf \
	fonts/URWBookman-Demi.otf \
	fonts/URWBookman-LightItalic.otf \
	fonts/URWBookman-Light.otf \
	fonts/URWGothic-BookOblique.otf \
	fonts/URWGothic-Book.otf \
	fonts/URWGothic-DemiOblique.otf \
	fonts/URWGothic-Demi.otf \
	fonts/Z003-MediumItalic.otf \
))

# vim: set noet sw=4 ts=4:
