# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,accessoriesgalleryphotoscelebration,$(call gb_UnpackedTarball_get_dir,gallery-pack)/accessories/Photos/Celebration))

$(eval $(call gb_Zip_add_dependencies,accessoriesgalleryphotoscelebration,\
	$(call gb_UnpackedTarball_get_target,gallery-pack) \
))

$(eval $(call gb_Zip_add_files,accessoriesgalleryphotoscelebration,\
	JS_PDPhO_fireworks_burst.jpg \
	JS_PDPhO_new_years_fireworks_explosion.jpg \
))

# vim: set noet sw=4 ts=4:
