# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,fonts_dejavu,font_dejavu))

$(eval $(call gb_ExternalPackage_set_outdir,fonts_dejavu,$(INSTROOT)))

$(eval $(call gb_ExternalPackage_add_unpacked_files,fonts_dejavu,$(LIBO_SHARE_FOLDER)/fonts/truetype,\
	ttf/DejaVuSans-Bold.ttf \
	ttf/DejaVuSans-BoldOblique.ttf \
	ttf/DejaVuSans-ExtraLight.ttf \
	ttf/DejaVuSans-Oblique.ttf \
	ttf/DejaVuSans.ttf \
	ttf/DejaVuSansCondensed-Bold.ttf \
	ttf/DejaVuSansCondensed-BoldOblique.ttf \
	ttf/DejaVuSansCondensed-Oblique.ttf \
	ttf/DejaVuSansCondensed.ttf \
	ttf/DejaVuSansMono-Bold.ttf \
	ttf/DejaVuSansMono-BoldOblique.ttf \
	ttf/DejaVuSansMono-Oblique.ttf \
	ttf/DejaVuSansMono.ttf \
	ttf/DejaVuSerif-Bold.ttf \
	ttf/DejaVuSerif-BoldItalic.ttf \
	ttf/DejaVuSerif-Italic.ttf \
	ttf/DejaVuSerif.ttf \
	ttf/DejaVuSerifCondensed-Bold.ttf \
	ttf/DejaVuSerifCondensed-BoldItalic.ttf \
	ttf/DejaVuSerifCondensed-Italic.ttf \
	ttf/DejaVuSerifCondensed.ttf \
))

# vim: set noet sw=4 ts=4:
