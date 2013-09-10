# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,extras_accessoriesgalleryphotoscities,gallery-pack))

$(eval $(call gb_ExternalPackage_set_outdir,extras_accessoriesgalleryphotoscities,$(gb_INSTROOT)))

$(eval $(call gb_ExternalPackage_add_unpacked_files,extras_accessoriesgalleryphotoscities,$(LIBO_SHARE_FOLDER)/gallery/Photos/Cities,\
	accessories/Photos/Cities/ArkSoft0507170040.jpg \
	accessories/Photos/Cities/ArkSoft0507170041.jpg \
	accessories/Photos/Cities/ArkSoft0508100003.jpg \
	accessories/Photos/Cities/ArkSoft0508100009.jpg \
	accessories/Photos/Cities/ArkSoft0508100010.jpg \
	accessories/Photos/Cities/ArkSoft0508100012.jpg \
	accessories/Photos/Cities/ArkSoft0508100017.jpg \
	accessories/Photos/Cities/morguefiles_com_pindiyath100_109777662243.jpg \
))

# vim: set noet sw=4 ts=4:
