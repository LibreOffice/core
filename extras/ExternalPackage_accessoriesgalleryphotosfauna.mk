# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,extras_accessoriesgalleryphotosfauna,gallery-pack))

$(eval $(call gb_ExternalPackage_set_outdir,extras_accessoriesgalleryphotosfauna,$(INSTROOT)))

$(eval $(call gb_ExternalPackage_add_unpacked_files,extras_accessoriesgalleryphotosfauna,$(LIBO_SHARE_FOLDER)/gallery/Photos/Fauna,\
	accessories/Photos/Fauna/ArkSoft0407030001.jpg \
	accessories/Photos/Fauna/bee_on_citrus.jpg \
	accessories/Photos/Fauna/dog01.jpg \
	accessories/Photos/Fauna/dog02.jpg \
	accessories/Photos/Fauna/dog_sv400114.jpg \
	accessories/Photos/Fauna/donkey_sv400131.jpg \
	accessories/Photos/Fauna/elephant01.jpg \
	accessories/Photos/Fauna/frog01.jpg \
	accessories/Photos/Fauna/frog02.jpg \
	accessories/Photos/Fauna/frog03.jpg \
	accessories/Photos/Fauna/frog04.jpg \
	accessories/Photos/Fauna/frog05.jpg \
	accessories/Photos/Fauna/jellies01.jpg \
	accessories/Photos/Fauna/JS_PDPhO_animal_duck.jpg \
	accessories/Photos/Fauna/JS_PDPhO_bee_hovering.jpg \
	accessories/Photos/Fauna/JS_PDPhO_bigsur_el_seals_seeweed.jpg \
	accessories/Photos/Fauna/JS_PDPhO_butterfly_on_leaf.jpg \
	accessories/Photos/Fauna/JS_PDPhO_ireland_sheep_shorn.jpg \
	accessories/Photos/Fauna/JS_PDPhO_spider_web.jpg \
	accessories/Photos/Fauna/kangaroo01.jpg \
	accessories/Photos/Fauna/koala01.jpg \
	accessories/Photos/Fauna/lizard01.jpg \
	accessories/Photos/Fauna/lizard02.jpg \
	accessories/Photos/Fauna/moluchus_horridus01.jpg \
	accessories/Photos/Fauna/moluchus_horridus02.jpg \
	accessories/Photos/Fauna/morguefiles_com_pindiyath100_109290559643.jpg \
	accessories/Photos/Fauna/morguefiles_com_pindiyath100_10964794274.jpg \
	accessories/Photos/Fauna/morguefiles_com_pindiyath100_110106397519.jpg \
	accessories/Photos/Fauna/morguefiles_com_pindiyath100_111023591.jpg \
	accessories/Photos/Fauna/morguefiles_com_pindiyath100_111478817614.jpg \
	accessories/Photos/Fauna/morguefiles_com_pindiyath100_121259975.jpg \
	accessories/Photos/Fauna/morguefiles_com_pindiyath100_121583357.jpg \
	accessories/Photos/Fauna/morguefiles_com_pindiyath100_121583377.jpg \
	accessories/Photos/Fauna/morguefiles_com_pindiyath100_128319370.jpg \
	accessories/Photos/Fauna/morguefiles_com_pindiyath100_129017506.jpg \
	accessories/Photos/Fauna/morguefiles_com_pindiyath100_A1200214.JPG \
	accessories/Photos/Fauna/parrot01.jpg \
	accessories/Photos/Fauna/penguin01.jpg \
	accessories/Photos/Fauna/raccoon.jpg \
	accessories/Photos/Fauna/snail.jpg \
	accessories/Photos/Fauna/spider01.jpg \
	accessories/Photos/Fauna/spider_sv400268.jpg \
	accessories/Photos/Fauna/spider_sv400270.jpg \
))

# vim: set noet sw=4 ts=4:
