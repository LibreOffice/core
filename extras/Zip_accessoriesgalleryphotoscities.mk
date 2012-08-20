# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,accessoriesgalleryphotoscities,$(call gb_UnpackedTarball_get_dir,gallery-pack)/accessories/Photos/Cities))

$(eval $(call gb_Zip_add_dependencies,accessoriesgalleryphotoscities,\
	$(call gb_UnpackedTarball_get_target,gallery-pack) \
))

$(eval $(call gb_Zip_add_files,accessoriesgalleryphotoscities,\
	ArkSoft0507170040.jpg \
	ArkSoft0507170041.jpg \
	ArkSoft0508100003.jpg \
	ArkSoft0508100009.jpg \
	ArkSoft0508100010.jpg \
	ArkSoft0508100012.jpg \
	ArkSoft0508100017.jpg \
	morguefiles_com_pindiyath100_109777662243.jpg \
))

# vim: set noet sw=4 ts=4:
