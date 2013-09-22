# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,extras_accessoriesgalleryphotosplants,gallery-pack))

$(eval $(call gb_ExternalPackage_set_outdir,extras_accessoriesgalleryphotosplants,$(INSTROOT)))

$(eval $(call gb_ExternalPackage_add_unpacked_files,extras_accessoriesgalleryphotosplants,$(LIBO_SHARE_FOLDER)/gallery/Photos/Plants,\
	accessories/Photos/Plants/ArkSoft0405020018.jpg \
	accessories/Photos/Plants/ArkSoft0405020019.jpg \
	accessories/Photos/Plants/ArkSoft0508100018.jpg \
	accessories/Photos/Plants/ArkSoft0508100020.jpg \
	accessories/Photos/Plants/autumn_bonsai.jpg \
	accessories/Photos/Plants/Banksia.jpg \
	accessories/Photos/Plants/blueberries_sv400141.jpg \
	accessories/Photos/Plants/dsc00002.jpg \
	accessories/Photos/Plants/dsc00304.jpg \
	accessories/Photos/Plants/dsc00324.jpg \
	accessories/Photos/Plants/dsc01427.jpg \
	accessories/Photos/Plants/dsc02182.jpg \
	accessories/Photos/Plants/dsc03552.jpg \
	accessories/Photos/Plants/dsc07896.jpg \
	accessories/Photos/Plants/dsc08539.jpg \
	accessories/Photos/Plants/dsc20040214_143702_43.jpg \
	accessories/Photos/Plants/dsc20040214_143714_44.jpg \
	accessories/Photos/Plants/dsc20040221_101357_28.jpg \
	accessories/Photos/Plants/dsc20040225_174051_2.jpg \
	accessories/Photos/Plants/dsc20040225_174205_4.jpg \
	accessories/Photos/Plants/dsc20040225_174257_7.jpg \
	accessories/Photos/Plants/dsc20040306_155810_46.jpg \
	accessories/Photos/Plants/dsc20040424_132536_49.jpg \
	accessories/Photos/Plants/dsc20040424_142153_9.jpg \
	accessories/Photos/Plants/dsc20040424_153430_50.jpg \
	accessories/Photos/Plants/dsc20040424_153647_56.jpg \
	accessories/Photos/Plants/dsc20040424_153758_59.jpg \
	accessories/Photos/Plants/dsc20040424_153816_60.jpg \
	accessories/Photos/Plants/dsc20040501_130922_3.jpg \
	accessories/Photos/Plants/dsc20040724_152027_45.jpg \
	accessories/Photos/Plants/dsc20040724_152233_49.jpg \
	accessories/Photos/Plants/dsc20040724_152306_50.jpg \
	accessories/Photos/Plants/dsc20040903_171512_16.jpg \
	accessories/Photos/Plants/dsc20041231_182903_42.jpg \
	accessories/Photos/Plants/dsc20041231_182927_43.jpg \
	accessories/Photos/Plants/dsc20041231_182947_45.jpg \
	accessories/Photos/Plants/dsc20050101_181121_21.jpg \
	accessories/Photos/Plants/dsc20050101_181808_30.jpg \
	accessories/Photos/Plants/dsc20050101_182126_34.jpg \
	accessories/Photos/Plants/dscn0269.jpg \
	accessories/Photos/Plants/dscn0528.jpg \
	accessories/Photos/Plants/edelweiss_sv400091.jpg \
	accessories/Photos/Plants/IMG_0824.jpg \
	accessories/Photos/Plants/IMG_4528.jpg \
	accessories/Photos/Plants/JS_PDPhO_fruit_still_life.jpg \
	accessories/Photos/Plants/JS_PDPhO_garlic_pepper.jpg \
	accessories/Photos/Plants/JS_PDPhO_onions.jpg \
	accessories/Photos/Plants/JS_PDPhO_pepper.jpg \
	accessories/Photos/Plants/morguefiles_com_pindiyath100_10929117394.jpg \
	accessories/Photos/Plants/morguefiles_com_pindiyath100_109292837223.jpg \
	accessories/Photos/Plants/morguefiles_com_pindiyath100_A1180933.JPG \
	accessories/Photos/Plants/morguefiles_com_pindiyath100_A1190412.JPG \
	accessories/Photos/Plants/morguefiles_com_pindiyath100_Green_leaves.JPG \
	accessories/Photos/Plants/morguefiles_com_pindiyath100_Kiwi_fruit.jpg \
	accessories/Photos/Plants/mushrooms_log.jpg \
	accessories/Photos/Plants/mushroom_sv400101.jpg \
	accessories/Photos/Plants/random_002.jpg \
	accessories/Photos/Plants/strawberries_sv400130.jpg \
	accessories/Photos/Plants/strawberry_sv400131.jpg \
))

# vim: set noet sw=4 ts=4:
