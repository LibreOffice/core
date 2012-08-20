# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,accessoriesgalleryphotostravel,$(call gb_UnpackedTarball_get_dir,gallery-pack)/accessories/Photos/Travel))

$(eval $(call gb_Zip_add_dependencies,accessoriesgalleryphotostravel,\
	$(call gb_UnpackedTarball_get_target,gallery-pack) \
))

$(eval $(call gb_Zip_add_files,accessoriesgalleryphotostravel,\
	JS_PDPhO_balloons_festival.jpg \
	JS_PDPhO_cruise_ship.jpg \
	JS_PDPhO_tall_ship.jpg \
	morguefiles_com_pindiyath100_10948812106.jpg \
	morguefiles_com_pindiyath100_130370322.jpg \
	morguefiles_com_pindiyath100_P1130277.JPG \
))

# vim: set noet sw=4 ts=4:
