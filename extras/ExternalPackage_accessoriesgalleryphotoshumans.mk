# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,extras_accessoriesgalleryphotoshumans,gallery-pack))

$(eval $(call gb_ExternalPackage_set_outdir,extras_accessoriesgalleryphotoshumans,$(gb_INSTROOT)))

$(eval $(call gb_ExternalPackage_add_unpacked_files,extras_accessoriesgalleryphotoshumans,$(LIBO_SHARE_FOLDER)/gallery/Photos/Humans,\
	accessories/Photos/Humans/1471425079_b3b53bdf5f_b.jpg \
	accessories/Photos/Humans/1472456323_62f1a7dad1_o.jpg \
	accessories/Photos/Humans/1473308070_30ac88194b_o.jpg \
	accessories/Photos/Humans/1481457389_86532082cb_b.jpg \
	accessories/Photos/Humans/1482314200_1f933fe66f_b.jpg \
	accessories/Photos/Humans/morguefiles_com_pindiyath100_A1200509.JPG \
	accessories/Photos/Humans/morguefiles_com_pindiyath100_A1200513.JPG \
	accessories/Photos/Humans/morguefiles_com_pindiyath100_A1220569.JPG \
	accessories/Photos/Humans/morguefiles_com_pindiyath100_Fun_time.JPG \
	accessories/Photos/Humans/morguefiles_com_pindiyath100_Lady_with_THE_UMBRELLA.jpg \
	accessories/Photos/Humans/morguefiles_com_pindiyath100_PIC1092760844134.jpg \
))

# vim: set noet sw=4 ts=4:
