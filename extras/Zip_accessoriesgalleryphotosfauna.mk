# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,accessoriesgalleryphotosfauna,$(call gb_UnpackedTarball_get_dir,gallery-pack)/accessories/Photos/Fauna))

$(eval $(call gb_Zip_add_dependencies,accessoriesgalleryphotosfauna,\
	$(call gb_UnpackedTarball_get_target,gallery-pack) \
))

$(eval $(call gb_Zip_add_files,accessoriesgalleryphotosfauna,\
	ArkSoft0407030001.jpg \
	bee_on_citrus.jpg \
	dog01.jpg \
	dog02.jpg \
	dog_sv400114.jpg \
	donkey_sv400131.jpg \
	elephant01.jpg \
	frog01.jpg \
	frog02.jpg \
	frog03.jpg \
	frog04.jpg \
	frog05.jpg \
	jellies01.jpg \
	JS_PDPhO_animal_duck.jpg \
	JS_PDPhO_bee_hovering.jpg \
	JS_PDPhO_bigsur_el_seals_seeweed.jpg \
	JS_PDPhO_butterfly_on_leaf.jpg \
	JS_PDPhO_ireland_sheep_shorn.jpg \
	JS_PDPhO_spider_web.jpg \
	kangaroo01.jpg \
	koala01.jpg \
	lizard01.jpg \
	lizard02.jpg \
	moluchus_horridus01.jpg \
	moluchus_horridus02.jpg \
	morguefiles_com_pindiyath100_109290559643.jpg \
	morguefiles_com_pindiyath100_10964794274.jpg \
	morguefiles_com_pindiyath100_110106397519.jpg \
	morguefiles_com_pindiyath100_111023591.jpg \
	morguefiles_com_pindiyath100_111478817614.jpg \
	morguefiles_com_pindiyath100_121259975.jpg \
	morguefiles_com_pindiyath100_121583357.jpg \
	morguefiles_com_pindiyath100_121583377.jpg \
	morguefiles_com_pindiyath100_128319370.jpg \
	morguefiles_com_pindiyath100_129017506.jpg \
	morguefiles_com_pindiyath100_A1200214.JPG \
	parrot01.jpg \
	penguin01.jpg \
	raccoon.jpg \
	snail.jpg \
	spider01.jpg \
	spider_sv400268.jpg \
	spider_sv400270.jpg \
))

# vim: set noet sw=4 ts=4:
