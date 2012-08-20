# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,accessoriesgalleryphotosstatues,$(call gb_UnpackedTarball_get_dir,gallery-pack)/accessories/Photos/Statues))

$(eval $(call gb_Zip_add_dependencies,accessoriesgalleryphotosstatues,\
	$(call gb_UnpackedTarball_get_target,gallery-pack) \
))

$(eval $(call gb_Zip_add_files,accessoriesgalleryphotosstatues,\
	ArkSoft0507170027.jpg \
	ArkSoft0507170032.jpg \
	ArkSoft0507170033.jpg \
	ArkSoft0507170045.jpg \
	ArkSoft0507170046.jpg \
))

# vim: set noet sw=4 ts=4:
