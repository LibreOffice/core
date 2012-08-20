# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,accessoriesgallerydrawspeople,$(call gb_UnpackedTarball_get_dir,gallery-pack)/accessories/Draws/People))

$(eval $(call gb_Zip_add_dependencies,accessoriesgallerydrawspeople,\
	$(call gb_UnpackedTarball_get_target,gallery-pack) \
))

$(eval $(call gb_Zip_add_files,accessoriesgallerydrawspeople,\
	Baboo1.png \
	Jac1.png \
	Jac2.png \
	Jac3.png \
	Jac4.png \
	Paul-1.png \
	Paul-2.png \
	Paul-3.jpg \
	Paul-3.png \
	Paul-4.png \
	Paul-5.png \
	Paul-6.png \
	Paul-7.png \
	Paul-8.png \
	Ruty1.png \
	tof-1.png \
	tof-2.png \
	tof-3.png \
	Zoa1.png \
	Zoa2.jpg \
))

# vim: set noet sw=4 ts=4:
