# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,font_dejavu_inc,$(call gb_UnpackedTarball_get_dir,font_dejavu)))

$(eval $(call gb_Package_use_unpacked,font_dejavu_inc,font_dejavu))

$(eval $(call gb_Package_add_files,font_dejavu_inc,pck,\
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

$(eval $(call gb_Package_add_file,font_dejavu_lic,pck/License_dejavu.txt,LICENSE))
