# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,accessoriesgalleryphotosflowers,$(call gb_UnpackedTarball_get_dir,gallery-pack)/accessories/Photos/Flowers))

$(eval $(call gb_Zip_add_dependencies,accessoriesgalleryphotosflowers,\
	$(call gb_UnpackedTarball_get_target,gallery-pack) \
))

$(eval $(call gb_Zip_add_files,accessoriesgalleryphotosflowers,\
	1481157185_28ea113529_o.jpg \
	1482010374_31c0e4c1ed_o.jpg \
	ArkSoft0405010001.jpg \
	ArkSoft0405020017.jpg \
	ArkSoft0405050005.jpg \
	ArkSoft0411070015.jpg \
	edelweiss_sv400091.jpg \
	JS_PDPhO_flowers_in_field_yellow_orange.jpg \
	JS_PDPhO_orchid_pink.jpg \
	JS_PDPhO_rose.jpg \
	JS_PDPhO_sunflower_in_sun.jpg \
	JS_PDPhO_water_lillies.jpg \
	morguefiles_com_pindiyath100_109292831129.jpg \
	morguefiles_com_pindiyath100_11005053248.jpg \
	morguefiles_com_pindiyath100_127103763.jpg \
	poppy_sv400105.jpg \
	rose_sv400109.jpg \
	rose_sv400157.jpg \
	spring_sv400013.jpg \
	spring_sv400015.jpg \
	spring_sv400017.jpg \
	spring_sv400033.jpg \
))

# vim: set noet sw=4 ts=4:
