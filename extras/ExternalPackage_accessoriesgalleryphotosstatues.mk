# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,extras_accessoriesgalleryphotosstatues,gallery-pack))

$(eval $(call gb_ExternalPackage_set_outdir,extras_accessoriesgalleryphotosstatues,$(INSTDIR)))

$(eval $(call gb_ExternalPackage_add_unpacked_files,extras_accessoriesgalleryphotosstatues,$(LIBO_SHARE_FOLDER)/gallery/Photos/Statues,\
	accessories/Photos/Statues/ArkSoft0507170027.jpg \
	accessories/Photos/Statues/ArkSoft0507170032.jpg \
	accessories/Photos/Statues/ArkSoft0507170033.jpg \
	accessories/Photos/Statues/ArkSoft0507170045.jpg \
	accessories/Photos/Statues/ArkSoft0507170046.jpg \
))

# vim: set noet sw=4 ts=4:
