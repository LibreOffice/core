# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,fonts_opensans,font_opensans))

$(eval $(call gb_ExternalPackage_add_unpacked_files,fonts_opensans,$(LIBO_SHARE_FOLDER)/fonts/truetype,\
	static/OpenSans_Condensed/OpenSans_Condensed-Bold.ttf \
	static/OpenSans_Condensed/OpenSans_Condensed-BoldItalic.ttf \
	static/OpenSans_Condensed/OpenSans_Condensed-ExtraBold.ttf \
	static/OpenSans_Condensed/OpenSans_Condensed-ExtraBoldItalic.ttf \
	static/OpenSans_Condensed/OpenSans_Condensed-Italic.ttf \
	static/OpenSans_Condensed/OpenSans_Condensed-Light.ttf \
	static/OpenSans_Condensed/OpenSans_Condensed-LightItalic.ttf \
	static/OpenSans_Condensed/OpenSans_Condensed-Medium.ttf \
	static/OpenSans_Condensed/OpenSans_Condensed-MediumItalic.ttf \
	static/OpenSans_Condensed/OpenSans_Condensed-Regular.ttf \
	static/OpenSans_Condensed/OpenSans_Condensed-SemiBold.ttf \
	static/OpenSans_Condensed/OpenSans_Condensed-SemiBoldItalic.ttf \
	static/OpenSans_SemiCondensed/OpenSans_SemiCondensed-Bold.ttf \
	static/OpenSans_SemiCondensed/OpenSans_SemiCondensed-BoldItalic.ttf \
	static/OpenSans_SemiCondensed/OpenSans_SemiCondensed-ExtraBold.ttf \
	static/OpenSans_SemiCondensed/OpenSans_SemiCondensed-ExtraBoldItalic.ttf \
	static/OpenSans_SemiCondensed/OpenSans_SemiCondensed-Italic.ttf \
	static/OpenSans_SemiCondensed/OpenSans_SemiCondensed-Light.ttf \
	static/OpenSans_SemiCondensed/OpenSans_SemiCondensed-LightItalic.ttf \
	static/OpenSans_SemiCondensed/OpenSans_SemiCondensed-Medium.ttf \
	static/OpenSans_SemiCondensed/OpenSans_SemiCondensed-MediumItalic.ttf \
	static/OpenSans_SemiCondensed/OpenSans_SemiCondensed-Regular.ttf \
	static/OpenSans_SemiCondensed/OpenSans_SemiCondensed-SemiBold.ttf \
	static/OpenSans_SemiCondensed/OpenSans_SemiCondensed-SemiBoldItalic.ttf \
	static/OpenSans/OpenSans-Bold.ttf \
	static/OpenSans/OpenSans-BoldItalic.ttf \
	static/OpenSans/OpenSans-ExtraBold.ttf \
	static/OpenSans/OpenSans-ExtraBoldItalic.ttf \
	static/OpenSans/OpenSans-Italic.ttf \
	static/OpenSans/OpenSans-Light.ttf \
	static/OpenSans/OpenSans-LightItalic.ttf \
	static/OpenSans/OpenSans-Medium.ttf \
	static/OpenSans/OpenSans-MediumItalic.ttf \
	static/OpenSans/OpenSans-Regular.ttf \
	static/OpenSans/OpenSans-SemiBold.ttf \
	static/OpenSans/OpenSans-SemiBoldItalic.ttf \
))

# vim: set noet sw=4 ts=4:
