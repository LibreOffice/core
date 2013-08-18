# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,extras_accessoriesgalleryphotosobjects,gallery-pack))

$(eval $(call gb_ExternalPackage_set_outdir,extras_accessoriesgalleryphotosobjects,$(INSTDIR)))

$(eval $(call gb_ExternalPackage_add_unpacked_files,extras_accessoriesgalleryphotosobjects,$(LIBO_SHARE_FOLDER)/gallery/Photos/Objects,\
	accessories/Photos/Objects/JS_PDPhO_dragon_celebration.jpg \
	accessories/Photos/Objects/morguefiles_com_pindiyath100_111064504847.jpg \
	accessories/Photos/Objects/morguefiles_com_pindiyath100_122931551.jpg \
	accessories/Photos/Objects/morguefiles_com_pindiyath100_148584163.jpg \
	accessories/Photos/Objects/morguefiles_com_pindiyath100_149059900.jpg \
	accessories/Photos/Objects/morguefiles_com_pindiyath100_149489898.jpg \
	accessories/Photos/Objects/morguefiles_com_pindiyath100_150855874.jpg \
	accessories/Photos/Objects/morguefiles_com_pindiyath100_160703628.jpg \
	accessories/Photos/Objects/morguefiles_com_pindiyath100_A1160419.JPG \
	accessories/Photos/Objects/morguefiles_com_pindiyath100_A1180678.JPG \
	accessories/Photos/Objects/morguefiles_com_pindiyath100_A1190005.jpg \
	accessories/Photos/Objects/morguefiles_com_pindiyath100_A1250317.JPG \
	accessories/Photos/Objects/morguefiles_com_pindiyath100_Fountain_pen.JPG \
	accessories/Photos/Objects/morguefiles_com_pindiyath100_P1120434.jpg \
	accessories/Photos/Objects/morguefiles_com_pindiyath100_PIC1092515922117.jpg \
	accessories/Photos/Objects/morguefiles_com_pindiyath100_PIC109263097871.jpg \
	accessories/Photos/Objects/morguefiles_com_pindiyath100_PIC109284443682.jpg \
	accessories/Photos/Objects/morguefiles_com_pindiyath100_Screw_Driver__and_screws.JPG \
))

# vim: set noet sw=4 ts=4:
