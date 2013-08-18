# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,extras_accessoriesgalleryphotosfoodsanddrinks,gallery-pack))

$(eval $(call gb_ExternalPackage_set_outdir,extras_accessoriesgalleryphotosfoodsanddrinks,$(INSTDIR)))

$(eval $(call gb_ExternalPackage_add_unpacked_files,extras_accessoriesgalleryphotosfoodsanddrinks,$(LIBO_SHARE_FOLDER)/gallery/Photos/FoodsandDrinks,\
	accessories/Photos/FoodsandDrinks/JS_PDPhO_beer.jpg \
	accessories/Photos/FoodsandDrinks/JS_PDPhO_fish_and_chips.jpg \
	accessories/Photos/FoodsandDrinks/JS_PDPhO_lasagna.jpg \
	accessories/Photos/FoodsandDrinks/JS_PDPhO_nachos.jpg \
	accessories/Photos/FoodsandDrinks/JS_PDPhO_pizza.jpg \
	accessories/Photos/FoodsandDrinks/JS_PDPhO_steaks.jpg \
	accessories/Photos/FoodsandDrinks/JS_PDPhO_strawberry_shortcake.jpg \
))

# vim: set noet sw=4 ts=4:
