# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,accessoriesgalleryphotosfoodsanddrinks,$(call gb_UnpackedTarball_get_dir,gallery-pack)/accessories/Photos/FoodsandDrinks))

$(eval $(call gb_Zip_add_dependencies,accessoriesgalleryphotosfoodsanddrinks,\
	$(call gb_UnpackedTarball_get_target,gallery-pack) \
))

$(eval $(call gb_Zip_add_files,accessoriesgalleryphotosfoodsanddrinks,\
	JS_PDPhO_beer.jpg \
	JS_PDPhO_fish_and_chips.jpg \
	JS_PDPhO_lasagna.jpg \
	JS_PDPhO_nachos.jpg \
	JS_PDPhO_pizza.jpg \
	JS_PDPhO_steaks.jpg \
	JS_PDPhO_strawberry_shortcake.jpg \
))

# vim: set noet sw=4 ts=4:
