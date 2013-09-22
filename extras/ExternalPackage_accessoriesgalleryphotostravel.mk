# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,extras_accessoriesgalleryphotostravel,gallery-pack))

$(eval $(call gb_ExternalPackage_set_outdir,extras_accessoriesgalleryphotostravel,$(INSTROOT)))

$(eval $(call gb_ExternalPackage_add_unpacked_files,extras_accessoriesgalleryphotostravel,$(LIBO_SHARE_FOLDER)/gallery/Photos/Travel,\
	accessories/Photos/Travel/JS_PDPhO_balloons_festival.jpg \
	accessories/Photos/Travel/JS_PDPhO_cruise_ship.jpg \
	accessories/Photos/Travel/JS_PDPhO_tall_ship.jpg \
	accessories/Photos/Travel/morguefiles_com_pindiyath100_10948812106.jpg \
	accessories/Photos/Travel/morguefiles_com_pindiyath100_130370322.jpg \
	accessories/Photos/Travel/morguefiles_com_pindiyath100_P1130277.JPG \
))

# vim: set noet sw=4 ts=4:
