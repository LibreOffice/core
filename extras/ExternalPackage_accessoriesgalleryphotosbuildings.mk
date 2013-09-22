# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,extras_accessoriesgalleryphotosbuildings,gallery-pack))

$(eval $(call gb_ExternalPackage_set_outdir,extras_accessoriesgalleryphotosbuildings,$(INSTROOT)))

$(eval $(call gb_ExternalPackage_add_unpacked_files,extras_accessoriesgalleryphotosbuildings,$(LIBO_SHARE_FOLDER)/gallery/Photos/Buildings,\
	accessories/Photos/Buildings/ArkSoft0408010009.jpg \
	accessories/Photos/Buildings/ArkSoft0408010023.jpg \
	accessories/Photos/Buildings/ArkSoft0408010026.jpg \
	accessories/Photos/Buildings/ArkSoft0507170031.jpg \
	accessories/Photos/Buildings/ArkSoft0508100004.jpg \
	accessories/Photos/Buildings/ArkSoft0508100005.jpg \
	accessories/Photos/Buildings/ArkSoft0508100006.jpg \
	accessories/Photos/Buildings/ArkSoft0508100007.jpg \
	accessories/Photos/Buildings/ArkSoft0508100008.jpg \
	accessories/Photos/Buildings/ArkSoft0508100011.jpg \
	accessories/Photos/Buildings/ArkSoft0508100013.jpg \
	accessories/Photos/Buildings/ArkSoft0508100014.jpg \
	accessories/Photos/Buildings/ArkSoft0508100015.jpg \
	accessories/Photos/Buildings/ArkSoft0508100016.jpg \
	accessories/Photos/Buildings/JS_PDPhO_ireland_cashel.jpg \
	accessories/Photos/Buildings/JS_PDPhO_ireland_rock_of_cashel.jpg \
	accessories/Photos/Buildings/JS_PDPhO_sandiego_building_into_clouds.jpg \
	accessories/Photos/Buildings/morguefiles_com_pindiyath100_10930048528.jpg \
	accessories/Photos/Buildings/morguefiles_com_pindiyath100_131512197.jpg \
	accessories/Photos/Buildings/morguefiles_com_pindiyath100_137771099.jpg \
))

# vim: set noet sw=4 ts=4:
