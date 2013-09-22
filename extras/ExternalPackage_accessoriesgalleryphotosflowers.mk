# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,extras_accessoriesgalleryphotosflowers,gallery-pack))

$(eval $(call gb_ExternalPackage_set_outdir,extras_accessoriesgalleryphotosflowers,$(INSTROOT)))

$(eval $(call gb_ExternalPackage_add_unpacked_files,extras_accessoriesgalleryphotosflowers,$(LIBO_SHARE_FOLDER)/gallery/Photos/Flowers,\
	accessories/Photos/Flowers/1481157185_28ea113529_o.jpg \
	accessories/Photos/Flowers/1482010374_31c0e4c1ed_o.jpg \
	accessories/Photos/Flowers/ArkSoft0405010001.jpg \
	accessories/Photos/Flowers/ArkSoft0405020017.jpg \
	accessories/Photos/Flowers/ArkSoft0405050005.jpg \
	accessories/Photos/Flowers/ArkSoft0411070015.jpg \
	accessories/Photos/Flowers/edelweiss_sv400091.jpg \
	accessories/Photos/Flowers/JS_PDPhO_flowers_in_field_yellow_orange.jpg \
	accessories/Photos/Flowers/JS_PDPhO_orchid_pink.jpg \
	accessories/Photos/Flowers/JS_PDPhO_rose.jpg \
	accessories/Photos/Flowers/JS_PDPhO_sunflower_in_sun.jpg \
	accessories/Photos/Flowers/JS_PDPhO_water_lillies.jpg \
	accessories/Photos/Flowers/morguefiles_com_pindiyath100_109292831129.jpg \
	accessories/Photos/Flowers/morguefiles_com_pindiyath100_11005053248.jpg \
	accessories/Photos/Flowers/morguefiles_com_pindiyath100_127103763.jpg \
	accessories/Photos/Flowers/poppy_sv400105.jpg \
	accessories/Photos/Flowers/rose_sv400109.jpg \
	accessories/Photos/Flowers/rose_sv400157.jpg \
	accessories/Photos/Flowers/spring_sv400013.jpg \
	accessories/Photos/Flowers/spring_sv400015.jpg \
	accessories/Photos/Flowers/spring_sv400017.jpg \
	accessories/Photos/Flowers/spring_sv400033.jpg \
))

# vim: set noet sw=4 ts=4:
