# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,accessoriesgalleryphotosbuildings,$(call gb_UnpackedTarball_get_dir,gallery-pack)/accessories/Photos/Buildings))

$(eval $(call gb_Zip_add_dependencies,accessoriesgalleryphotosbuildings,\
	$(call gb_UnpackedTarball_get_target,gallery-pack) \
))

$(eval $(call gb_Zip_add_files,accessoriesgalleryphotosbuildings,\
	ArkSoft0408010009.jpg \
	ArkSoft0408010023.jpg \
	ArkSoft0408010026.jpg \
	ArkSoft0507170031.jpg \
	ArkSoft0508100004.jpg \
	ArkSoft0508100005.jpg \
	ArkSoft0508100006.jpg \
	ArkSoft0508100007.jpg \
	ArkSoft0508100008.jpg \
	ArkSoft0508100011.jpg \
	ArkSoft0508100013.jpg \
	ArkSoft0508100014.jpg \
	ArkSoft0508100015.jpg \
	ArkSoft0508100016.jpg \
	JS_PDPhO_ireland_cashel.jpg \
	JS_PDPhO_ireland_rock_of_cashel.jpg \
	JS_PDPhO_sandiego_building_into_clouds.jpg \
	morguefiles_com_pindiyath100_10930048528.jpg \
	morguefiles_com_pindiyath100_131512197.jpg \
	morguefiles_com_pindiyath100_137771099.jpg \
))

# vim: set noet sw=4 ts=4:
