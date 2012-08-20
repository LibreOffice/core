# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,accessoriesgalleryphotosobjects,$(call gb_UnpackedTarball_get_dir,gallery-pack)/accessories/Photos/Objects))

$(eval $(call gb_Zip_add_dependencies,accessoriesgalleryphotosobjects,\
	$(call gb_UnpackedTarball_get_target,gallery-pack) \
))

$(eval $(call gb_Zip_add_files,accessoriesgalleryphotosobjects,\
	JS_PDPhO_dragon_celebration.jpg \
	morguefiles_com_pindiyath100_111064504847.jpg \
	morguefiles_com_pindiyath100_122931551.jpg \
	morguefiles_com_pindiyath100_148584163.jpg \
	morguefiles_com_pindiyath100_149059900.jpg \
	morguefiles_com_pindiyath100_149489898.jpg \
	morguefiles_com_pindiyath100_150855874.jpg \
	morguefiles_com_pindiyath100_160703628.jpg \
	morguefiles_com_pindiyath100_A1160419.JPG \
	morguefiles_com_pindiyath100_A1180678.JPG \
	morguefiles_com_pindiyath100_A1190005.jpg \
	morguefiles_com_pindiyath100_A1250317.JPG \
	morguefiles_com_pindiyath100_Fountain_pen.JPG \
	morguefiles_com_pindiyath100_P1120434.jpg \
	morguefiles_com_pindiyath100_PIC1092515922117.jpg \
	morguefiles_com_pindiyath100_PIC109263097871.jpg \
	morguefiles_com_pindiyath100_PIC109284443682.jpg \
	morguefiles_com_pindiyath100_Screw_Driver__and_screws.JPG \
))

# vim: set noet sw=4 ts=4:
