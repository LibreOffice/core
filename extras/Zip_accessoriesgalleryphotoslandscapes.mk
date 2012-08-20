# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,accessoriesgalleryphotoslandscapes,$(call gb_UnpackedTarball_get_dir,gallery-pack)/accessories/Photos/Landscapes))

$(eval $(call gb_Zip_add_dependencies,accessoriesgalleryphotoslandscapes,\
	$(call gb_UnpackedTarball_get_target,gallery-pack) \
))

$(eval $(call gb_Zip_add_files,accessoriesgalleryphotoslandscapes,\
	ArkSoft0405010010.jpg \
	ArkSoft0405010019.jpg \
	ArkSoft0405010050.jpg \
	ArkSoft0405020009.jpg \
	ArkSoft0405020012.jpg \
	ArkSoft0405050001.jpg \
	ArkSoft0405050016.jpg \
	ArkSoft0407030007.jpg \
	ArkSoft0411070003.jpg \
	ArkSoft0411070008.jpg \
	ArkSoft0411070009.jpg \
	ArkSoft0411070011.jpg \
	ArkSoft0411070025.jpg \
	ArkSoft0411070029.jpg \
	ArkSoft0508100001.jpg \
	ArkSoft0508100002.jpg \
	ArkSoft0508100019.jpg \
	dsc00001.jpg \
	dsc00002.jpg \
	dsc00003.jpg \
	dsc00004_001.jpg \
	dsc00004_002.jpg \
	dsc00004.jpg \
	dsc00005_001.jpg \
	dsc00005_002.jpg \
	dsc00006_001.jpg \
	dsc00006.jpg \
	dsc00012.jpg \
	dsc00021.jpg \
	dsc00029_001.jpg \
	dsc00038.jpg \
	dsc00039.jpg \
	dsc00041.jpg \
	dsc00043.jpg \
	dsc00045.jpg \
	JS_PDPhO_arizona_monument_valley.jpg \
	JS_PDPhO_bigsur_crashing_waves.jpg \
	JS_PDPhO_bigsur_mcway_falls.jpg \
	JS_PDPhO_canada_banff.jpg \
	JS_PDPhO_canada_moraine_lake_near_lake_louise.jpg \
	JS_PDPhO_grand_canyon_3.jpg \
	JS_PDPhO_grand_canyon_with_snow.jpg \
	JS_PDPhO_imperial_beach_pier.jpg \
	JS_PDPhO_ireland_beach_sheep.jpg \
	JS_PDPhO_ireland_glencar_falls.jpg \
	JS_PDPhO_ocean_beach_surf.jpg \
	JS_PDPhO_point_loma_tide_pools.jpg \
	lake_sv400007.jpg \
	lake_sv400009.jpg \
	lake_sv400202.jpg \
	morguefiles_com_pindiyath100_10943568670.jpg \
	morguefiles_com_pindiyath100_132583019.jpg \
	morguefiles_com_pindiyath100_143459647.jpg \
	morguefiles_com_pindiyath100_A1190465a.JPG \
	morguefiles_com_pindiyath100_A12202723.JPG \
	morguefiles_com_pindiyath100_Garden.JPG \
	mountains_sv400026.jpg \
	mountains_sv400073.jpg \
	spring_sv400040.jpg \
))

# vim: set noet sw=4 ts=4:
