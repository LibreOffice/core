# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,accessoriesgalleryphotosplants,$(call gb_UnpackedTarball_get_dir,gallery-pack)/accessories/Photos/Plants))

$(eval $(call gb_Zip_add_dependencies,accessoriesgalleryphotosplants,\
	$(call gb_UnpackedTarball_get_target,gallery-pack) \
))

$(eval $(call gb_Zip_add_files,accessoriesgalleryphotosplants,\
	ArkSoft0405020018.jpg \
	ArkSoft0405020019.jpg \
	ArkSoft0508100018.jpg \
	ArkSoft0508100020.jpg \
	autumn_bonsai.jpg \
	Banksia.jpg \
	blueberries_sv400141.jpg \
	dsc00002.jpg \
	dsc00304.jpg \
	dsc00324.jpg \
	dsc01427.jpg \
	dsc02182.jpg \
	dsc03552.jpg \
	dsc07896.jpg \
	dsc08539.jpg \
	dsc20040214_143702_43.jpg \
	dsc20040214_143714_44.jpg \
	dsc20040221_101357_28.jpg \
	dsc20040225_174051_2.jpg \
	dsc20040225_174205_4.jpg \
	dsc20040225_174257_7.jpg \
	dsc20040306_155810_46.jpg \
	dsc20040424_132536_49.jpg \
	dsc20040424_142153_9.jpg \
	dsc20040424_153430_50.jpg \
	dsc20040424_153647_56.jpg \
	dsc20040424_153758_59.jpg \
	dsc20040424_153816_60.jpg \
	dsc20040501_130922_3.jpg \
	dsc20040724_152027_45.jpg \
	dsc20040724_152233_49.jpg \
	dsc20040724_152306_50.jpg \
	dsc20040903_171512_16.jpg \
	dsc20041231_182903_42.jpg \
	dsc20041231_182927_43.jpg \
	dsc20041231_182947_45.jpg \
	dsc20050101_181121_21.jpg \
	dsc20050101_181808_30.jpg \
	dsc20050101_182126_34.jpg \
	dscn0269.jpg \
	dscn0528.jpg \
	edelweiss_sv400091.jpg \
	IMG_0824.jpg \
	IMG_4528.jpg \
	JS_PDPhO_fruit_still_life.jpg \
	JS_PDPhO_garlic_pepper.jpg \
	JS_PDPhO_onions.jpg \
	JS_PDPhO_pepper.jpg \
	morguefiles_com_pindiyath100_10929117394.jpg \
	morguefiles_com_pindiyath100_109292837223.jpg \
	morguefiles_com_pindiyath100_A1180933.JPG \
	morguefiles_com_pindiyath100_A1190412.JPG \
	morguefiles_com_pindiyath100_Green_leaves.JPG \
	morguefiles_com_pindiyath100_Kiwi_fruit.jpg \
	mushrooms_log.jpg \
	mushroom_sv400101.jpg \
	random_002.jpg \
	strawberries_sv400130.jpg \
	strawberry_sv400131.jpg \
))

# vim: set noet sw=4 ts=4:
