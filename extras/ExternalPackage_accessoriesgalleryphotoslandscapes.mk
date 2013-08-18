# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,extras_accessoriesgalleryphotoslandscapes,gallery-pack))

$(eval $(call gb_ExternalPackage_set_outdir,extras_accessoriesgalleryphotoslandscapes,$(INSTDIR)))

$(eval $(call gb_ExternalPackage_add_unpacked_files,extras_accessoriesgalleryphotoslandscapes,$(LIBO_SHARE_FOLDER)/gallery/Photos/Landscapes,\
	accessories/Photos/Landscapes/ArkSoft0405010010.jpg \
	accessories/Photos/Landscapes/ArkSoft0405010019.jpg \
	accessories/Photos/Landscapes/ArkSoft0405010050.jpg \
	accessories/Photos/Landscapes/ArkSoft0405020009.jpg \
	accessories/Photos/Landscapes/ArkSoft0405020012.jpg \
	accessories/Photos/Landscapes/ArkSoft0405050001.jpg \
	accessories/Photos/Landscapes/ArkSoft0405050016.jpg \
	accessories/Photos/Landscapes/ArkSoft0407030007.jpg \
	accessories/Photos/Landscapes/ArkSoft0411070003.jpg \
	accessories/Photos/Landscapes/ArkSoft0411070008.jpg \
	accessories/Photos/Landscapes/ArkSoft0411070009.jpg \
	accessories/Photos/Landscapes/ArkSoft0411070011.jpg \
	accessories/Photos/Landscapes/ArkSoft0411070025.jpg \
	accessories/Photos/Landscapes/ArkSoft0411070029.jpg \
	accessories/Photos/Landscapes/ArkSoft0508100001.jpg \
	accessories/Photos/Landscapes/ArkSoft0508100002.jpg \
	accessories/Photos/Landscapes/ArkSoft0508100019.jpg \
	accessories/Photos/Landscapes/dsc00001.jpg \
	accessories/Photos/Landscapes/dsc00002.jpg \
	accessories/Photos/Landscapes/dsc00003.jpg \
	accessories/Photos/Landscapes/dsc00004_001.jpg \
	accessories/Photos/Landscapes/dsc00004_002.jpg \
	accessories/Photos/Landscapes/dsc00004.jpg \
	accessories/Photos/Landscapes/dsc00005_001.jpg \
	accessories/Photos/Landscapes/dsc00005_002.jpg \
	accessories/Photos/Landscapes/dsc00006_001.jpg \
	accessories/Photos/Landscapes/dsc00006.jpg \
	accessories/Photos/Landscapes/dsc00012.jpg \
	accessories/Photos/Landscapes/dsc00021.jpg \
	accessories/Photos/Landscapes/dsc00029_001.jpg \
	accessories/Photos/Landscapes/dsc00038.jpg \
	accessories/Photos/Landscapes/dsc00039.jpg \
	accessories/Photos/Landscapes/dsc00041.jpg \
	accessories/Photos/Landscapes/dsc00043.jpg \
	accessories/Photos/Landscapes/dsc00045.jpg \
	accessories/Photos/Landscapes/JS_PDPhO_arizona_monument_valley.jpg \
	accessories/Photos/Landscapes/JS_PDPhO_bigsur_crashing_waves.jpg \
	accessories/Photos/Landscapes/JS_PDPhO_bigsur_mcway_falls.jpg \
	accessories/Photos/Landscapes/JS_PDPhO_canada_banff.jpg \
	accessories/Photos/Landscapes/JS_PDPhO_canada_moraine_lake_near_lake_louise.jpg \
	accessories/Photos/Landscapes/JS_PDPhO_grand_canyon_3.jpg \
	accessories/Photos/Landscapes/JS_PDPhO_grand_canyon_with_snow.jpg \
	accessories/Photos/Landscapes/JS_PDPhO_imperial_beach_pier.jpg \
	accessories/Photos/Landscapes/JS_PDPhO_ireland_beach_sheep.jpg \
	accessories/Photos/Landscapes/JS_PDPhO_ireland_glencar_falls.jpg \
	accessories/Photos/Landscapes/JS_PDPhO_ocean_beach_surf.jpg \
	accessories/Photos/Landscapes/JS_PDPhO_point_loma_tide_pools.jpg \
	accessories/Photos/Landscapes/lake_sv400007.jpg \
	accessories/Photos/Landscapes/lake_sv400009.jpg \
	accessories/Photos/Landscapes/lake_sv400202.jpg \
	accessories/Photos/Landscapes/morguefiles_com_pindiyath100_10943568670.jpg \
	accessories/Photos/Landscapes/morguefiles_com_pindiyath100_132583019.jpg \
	accessories/Photos/Landscapes/morguefiles_com_pindiyath100_143459647.jpg \
	accessories/Photos/Landscapes/morguefiles_com_pindiyath100_A1190465a.JPG \
	accessories/Photos/Landscapes/morguefiles_com_pindiyath100_A12202723.JPG \
	accessories/Photos/Landscapes/morguefiles_com_pindiyath100_Garden.JPG \
	accessories/Photos/Landscapes/mountains_sv400026.jpg \
	accessories/Photos/Landscapes/mountains_sv400073.jpg \
	accessories/Photos/Landscapes/spring_sv400040.jpg \
))

# vim: set noet sw=4 ts=4:
