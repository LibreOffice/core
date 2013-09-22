# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,extras_accessoriesgalleryphotoscelebration,gallery-pack))

$(eval $(call gb_ExternalPackage_set_outdir,extras_accessoriesgalleryphotoscelebration,$(INSTROOT)))

$(eval $(call gb_ExternalPackage_add_unpacked_files,extras_accessoriesgalleryphotoscelebration,$(LIBO_SHARE_FOLDER)/gallery/Photos/Celebration,\
	accessories/Photos/Celebration/JS_PDPhO_fireworks_burst.jpg \
	accessories/Photos/Celebration/JS_PDPhO_new_years_fireworks_explosion.jpg \
))

# vim: set noet sw=4 ts=4:
