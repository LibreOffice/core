# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

#$(eval $(call gb_Package_Package,extras_accessoriesgallerydrawspeople,$(call gb_UnpackedTarball_get_dir,gallery-pack)/accessories/Draws/People))
$(eval $(call gb_ExternalPackage_ExternalPackage,extras_accessoriesgallerydrawspeople,gallery-pack))

$(eval $(call gb_ExternalPackage_set_outdir,extras_accessoriesgallerydrawspeople,$(INSTROOT)))

$(eval $(call gb_ExternalPackage_add_unpacked_files,extras_accessoriesgallerydrawspeople,$(LIBO_SHARE_FOLDER)/gallery/Draws/People,\
	accessories/Draws/People/Baboo1.png \
	accessories/Draws/People/Jac1.png \
	accessories/Draws/People/Jac2.png \
	accessories/Draws/People/Jac3.png \
	accessories/Draws/People/Jac4.png \
	accessories/Draws/People/Paul-1.png \
	accessories/Draws/People/Paul-2.png \
	accessories/Draws/People/Paul-3.jpg \
	accessories/Draws/People/Paul-3.png \
	accessories/Draws/People/Paul-4.png \
	accessories/Draws/People/Paul-5.png \
	accessories/Draws/People/Paul-6.png \
	accessories/Draws/People/Paul-7.png \
	accessories/Draws/People/Paul-8.png \
	accessories/Draws/People/Ruty1.png \
	accessories/Draws/People/tof-1.png \
	accessories/Draws/People/tof-2.png \
	accessories/Draws/People/tof-3.png \
	accessories/Draws/People/Zoa1.png \
	accessories/Draws/People/Zoa2.jpg \
))

# vim: set noet sw=4 ts=4:
