# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,accessoriesgallerynonfree,$(call gb_UnpackedTarball_get_dir,gallery-pack)/accessories-nonfree))

$(eval $(call gb_Zip_add_dependencies,accessoriesgallerynonfree,\
	$(call gb_UnpackedTarball_get_target,gallery-pack) \
))

$(eval $(call gb_Zip_add_files,accessoriesgallerynonfree,\
	sg600.sdg \
	sg600.sdv \
	sg600.thm \
	sg601.sdg \
	sg601.sdv \
	sg601.thm \
	sg602.sdg \
	sg602.sdv \
	sg602.thm \
	sg603.sdg \
	sg603.sdv \
	sg603.thm \
))

# vim: set noet sw=4 ts=4:
