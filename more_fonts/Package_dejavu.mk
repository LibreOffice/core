# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,font_dejavu_inc,$(call gb_UnpackedTarball_get_dir,font_dejavu/ttf)))

$(eval $(call gb_Package_use_unpacked,font_dejavu_inc,font_dejavu))

$(eval $(call gb_Package_add_files,font_dejavu_inc,pck,\
	DejaVuSans-Bold.ttf \
	DejaVuSans-BoldOblique.ttf \
	DejaVuSans-ExtraLight.ttf \
	DejaVuSans-Oblique.ttf \
	DejaVuSans.ttf \
	DejaVuSansCondensed-Bold.ttf \
	DejaVuSansCondensed-BoldOblique.ttf \
	DejaVuSansCondensed-Oblique.ttf \
	DejaVuSansCondensed.ttf \
	DejaVuSansMono-Bold.ttf \
	DejaVuSansMono-BoldOblique.ttf \
	DejaVuSansMono-Oblique.ttf \
	DejaVuSansMono.ttf \
	DejaVuSerif-Bold.ttf \
	DejaVuSerif-BoldItalic.ttf \
	DejaVuSerif-Italic.ttf \
	DejaVuSerif.ttf \
	DejaVuSerifCondensed-Bold.ttf \
	DejaVuSerifCondensed-BoldItalic.ttf \
	DejaVuSerifCondensed-Italic.ttf \
	DejaVuSerifCondensed.ttf \
))


$(eval $(call gb_Package_Package,font_dejavu_lic,$(call gb_UnpackedTarball_get_dir,font_dejavu/)))
$(eval $(call gb_Package_use_unpacked,font_dejavu_lic,font_dejavu))
$(eval $(call gb_Package_add_file,font_dejavu_lic,pck/License_dejavu.txt,LICENSE))
